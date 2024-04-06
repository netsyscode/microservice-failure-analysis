package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io/ioutil"
	"log"
	"my_modules/module_src/bpfwrapper"
	"my_modules/module_src/privileges"
	"my_modules/module_src/settings"
	"os"
	"os/signal"
	"runtime"
	"syscall"
	"time"
	"unsafe"

	"github.com/alexflint/go-arg"
	"github.com/iovisor/gobpf/bcc"
)

const headerFieldStrSize = 128

type headerField struct {
	Size uint32
	Msg  [headerFieldStrSize]byte
}

// http2HeaderEvent's memory layout is identical to the go_grpc_http2_header_event_t in bpf_program.go, such that the
// event data obtained from the perf buffer can be directly copied to http2HeaderEvent.
type http2HeaderEvent struct {
	Name  headerField
	Value headerField
}

var (
	hooks = []bpfwrapper.Uprobe{
		{
			FunctionToHook: "SSL_write",
			HookName:       "probe_entry_ssl_write",
			Type:           bpfwrapper.EntryType,
		},
		{
			FunctionToHook: "SSL_write",
			HookName:       "probe_ret_ssl_write",
			Type:           bpfwrapper.ReturnType,
		},
		{
			FunctionToHook: "SSL_read",
			HookName:       "probe_entry_ssl_read",
			Type:           bpfwrapper.EntryType,
		},
		{
			FunctionToHook: "SSL_read",
			HookName:       "probe_ret_ssl_read",
			Type:           bpfwrapper.ReturnType,
		},
		{
			FunctionToHook: "SSL_write_ex",
			HookName:       "probe_entry_ssl_write",
			Type:           bpfwrapper.EntryType,
		},
		{
			FunctionToHook: "SSL_write_ex",
			HookName:       "probe_ret_ssl_write",
			Type:           bpfwrapper.ReturnType,
		},
		{
			FunctionToHook: "SSL_read_ex",
			HookName:       "probe_entry_ssl_read",
			Type:           bpfwrapper.EntryType,
		},
		{
			FunctionToHook: "SSL_read_ex",
			HookName:       "probe_ret_ssl_read",
			Type:           bpfwrapper.ReturnType,
		},
		// {
		// 	FunctionToHook: "google.golang.org/grpc/internal/transport.(*loopyWriter).writeHeader",
		// 	HookName:       "probe_ret_ssl_read",
		// 	Type:           bpfwrapper.EntryType,
		// },
	}
)

// args represents the command line arguments.
var args struct {
	BPFFile string `arg:"required,positional"`
	PID     int    `arg:"--pid" default:"-1"`
}

// DataEvent is a conversion of the following C-Struct into GO.
//
//	struct data_event_t {
//		uint64_t timestamp_ns;
//		uint32_t pid;
//		enum traffic_direction_t direction;
//		char msg [400];
//	};
type DataEvent struct {
	TimestampNano uint64
	PID           uint32
	Direction     int32
	Buffer        [400]byte
}

func formatHeaderField(field headerField) string {
	return string(field.Msg[0:field.Size])
}

func formatHeaderEvent(event http2HeaderEvent) string {
	return fmt.Sprintf("{ Header_Name:'%s', Header_Value='%s' }", formatHeaderField(event.Name), formatHeaderField(event.Value))
}

func openEventCallback(inputChan chan []byte) {
	for data := range inputChan {
		if data == nil {
			return
		}
		var event DataEvent

		if err := binary.Read(bytes.NewReader(data), bcc.GetHostByteOrder(), &event); err != nil {
			log.Printf("Failed to decode received data: %+v", err)
			continue
		}

		event.TimestampNano += settings.GetRealTimeOffset()
		if event.Direction == 0 { // egress
			fmt.Printf("----------------------------------------\nResponse to client {pid: %v, time: %v, buffer: %s}\n", event.PID, time.Unix(0, int64(event.TimestampNano)), string(event.Buffer[:]))
		} else {
			fmt.Printf("----------------------------------------\nRequest from client {pid: %v, time: %v, buffer: %s}\n", event.PID, time.Unix(0, int64(event.TimestampNano)), string(event.Buffer[:]))
		}
	}
}

func openEventCallback_new(inputChan chan []byte) {
	for data := range inputChan {
		if data == nil {
			return
		}
		var parsed http2HeaderEvent
		if err := binary.Read(bytes.NewReader(data), bcc.GetHostByteOrder(), &parsed); err != nil {
			panic(err)
		}
		fmt.Println(formatHeaderEvent(parsed))
	}
	fmt.Printf("----------------------------------------\n")
}

var (
	numberOfCPUs = runtime.NumCPU()
)

func fillPerCPUArray(bpfModule *bcc.Module, arrayName string, key int, value int) error {
	arr := make([]int, numberOfCPUs)
	for i := 0; i < numberOfCPUs; i++ {
		arr[i] = value
	}

	controlValues := bcc.NewTable(bpfModule.TableId(arrayName), bpfModule)
	return controlValues.SetP(unsafe.Pointer(&key), unsafe.Pointer(&arr[0]))
}

func mustAttachUprobe(bccMod *bcc.Module, binaryProg, symbol, probeFn string) {
	uprobeFD, err := bccMod.LoadUprobe(probeFn)
	if err != nil {
		panic(err)
	}
	err = bccMod.AttachUprobe(binaryProg, symbol, uprobeFD, -1 /*pid*/)
	if err != nil {
		panic(err)
	}
}

// func mustAttachUretprobe(bccMod *bcc.Module, binaryProg, symbol, probeFn string) {
// 	uprobeFD, err := bccMod.LoadUprobe(probeFn)
// 	if err != nil {
// 		panic(err)
// 	}
// 	err = bccMod.AttachUretprobe(binaryProg, symbol, uprobeFD, -1 /*pid*/)
// 	if err != nil {
// 		panic(err)
// 	}
// }

func main() {
	arg.MustParse(&args)

	bpfSourceCodeContent, err := ioutil.ReadFile(args.BPFFile)
	if err != nil {
		log.Panic(err)
	}

	defer privileges.RecoverFromCrashes()
	privileges.AbortIfNotRoot()

	if err := settings.InitRealTimeOffset(); err != nil {
		log.Printf("Failed fixing BPF clock, timings will be offseted: %v", err)
	}

	// Catching all termination signals to perform a cleanup when being stopped.
	sig := make(chan os.Signal, 1)
	signal.Notify(sig, syscall.SIGHUP, syscall.SIGINT, syscall.SIGQUIT, syscall.SIGTERM)

	bpfModule := bcc.NewModule(string(bpfSourceCodeContent), nil)
	if bpfModule == nil {
		log.Panic("bpf is nil")
	}
	defer bpfModule.Close()

	callbacks := []*bpfwrapper.ProbeChannel{bpfwrapper.NewProbeChannel("data_events", openEventCallback)}

	callbacks_new := []*bpfwrapper.ProbeChannel{bpfwrapper.NewProbeChannel("go_http2_header_events", openEventCallback_new)}

	if err := bpfwrapper.LaunchPerfBufferConsumers(bpfModule, callbacks); err != nil {
		log.Panic(err)
	}

	if err := bpfwrapper.LaunchPerfBufferConsumers(bpfModule, callbacks_new); err != nil {
		log.Panic(err)
	}

	if err := fillPerCPUArray(bpfModule, "pid", 0, args.PID); err != nil {
		log.Panic(err)
	}

	if err := bpfwrapper.AttachUprobes("/usr/lib/x86_64-linux-gnu/libssl.so.3", args.PID, bpfModule, hooks); err != nil {
		log.Panic(err)
	}

	const loopWriterWriteHeaderSymbol = "google.golang.org/grpc/internal/transport.(*loopyWriter).writeHeader"
	const loopWriterWriteHeaderProbeFn = "probe_loopy_writer_write_header"
	mustAttachUprobe(bpfModule, "/tmp/grpc_server", loopWriterWriteHeaderSymbol, loopWriterWriteHeaderProbeFn)

	const http2ServerOperateHeadersSymbol = "google.golang.org/grpc/internal/transport.(*http2Server).operateHeaders"
	const http2ServerOperateHeadersProbeFn = "probe_http2_server_operate_headers"
	mustAttachUprobe(bpfModule, "/tmp/grpc_server", http2ServerOperateHeadersSymbol, http2ServerOperateHeadersProbeFn)

	log.Println("Sniffer is ready\n----------------------------------------")
	<-sig
	log.Println("Signaled to terminate")
}
