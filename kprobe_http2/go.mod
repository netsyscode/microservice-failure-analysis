module kprobe_http1_modules

go 1.22

replace kprobe_http1_modules => ./

require (
	github.com/iovisor/gobpf v0.2.0
	golang.org/x/sys v0.0.0-20220227234510-4e6760a101f9
)
