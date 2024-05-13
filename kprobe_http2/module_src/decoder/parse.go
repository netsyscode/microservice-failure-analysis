package decoder

import (
    // "encoding/hex"
    "fmt"
    "strconv"
	"strings"
	"bytes"
)

func hexStringsToBytes(hexStrings []string) ([]byte) {
    var bytes []byte
    for _, hex := range hexStrings {
        // Parse the hex string into a uint64, but we only want a byte
        b, err := strconv.ParseUint(hex, 16, 8) // base 16 for hex, 8 bits for byte
        if err != nil {
            return nil // Return an error if any hex string is not a valid hex
        }
        bytes = append(bytes, byte(b)) // Cast to byte and append to the slice
    }
    return bytes
}

func ParseHTTP2Bytes(hexInput string) {
    hexValues := splitHex(hexInput)
    // Skip the connection preface if present
    preface := []string{"50", "52", "49", "20", "2a", "20", "48", "54", "54", "50", "2f", "32", "2e", "30", "0d", "0a", "0d", "0a", "53", "4d", "0d", "0a", "0d", "0a"}
    if equalSlice(hexValues[:24], preface) {
        hexValues = hexValues[24:]
    }

    frameIndex := 0

    for {
        if frameIndex+9 >= len(hexValues) {
            break
        }

        frameHeader := hexValues[frameIndex : frameIndex+9]
        frameLength, _ := strconv.ParseInt(concatHex(frameHeader[:3]), 16, 32)
        frameType, _ := strconv.ParseInt(frameHeader[3], 16, 32)
        frameStreamID, _ := strconv.ParseInt(concatHex(frameHeader[5:9]), 16, 32)

        if frameStreamID == 0 {
            frameIndex += 9 + int(frameLength)
            continue
        }

        switch frameType {
        case 0x0:
            // fmt.Println("DATA frame")
			// TODO: analyze data payload
            // padded := 0
            // padLength := 0
            // flags, _ := strconv.ParseInt(frameHeader[4], 16, 32)
            // if flags&0x8 != 0 {
            //     padLength, _ = strconv.Atoi(hexValues[frameIndex+9])
            //     padLength += 1
            //     padded = 1
            // }
            // dataPayload := hexValues[frameIndex+9+padded : frameIndex+9+int(frameLength)-padLength]
            // fmt.Println("Data payload:", dataPayload)

        case 0x1:
            // fmt.Println("HEADERS frame")
			padded := 0
			padLength := 0
			priorityLength := 0 // Length of the priority field, if present
			flags, _ := strconv.ParseInt(frameHeader[4], 16, 32)
			
			// Check if padded flag is set
			if flags&0x8 != 0 {
				padLength, _ = strconv.Atoi(hexValues[frameIndex+9]) // Get the pad length from the first byte after the header
				padLength += 1 // Include the length byte itself in the total padding
				padded = 1
			}
			
			// Check if priority flag is set
			if flags&0x20 != 0 {
				priorityLength = 5 // The priority field takes 5 bytes
			}
			
			// Calculate the start and end of the headers block
			headersStart := frameIndex + 9 + padded + priorityLength
			headersEnd := frameIndex + 9 + int(frameLength) - padLength
			
			// Extract the headers payload
			headersPayload := hexValues[headersStart:headersEnd]
			
			// Placeholder: Analyze headers payload or convert it to readable format
			analyzedResult := analyzeHeadersPayload(hexStringsToBytes(headersPayload)) // Define this function to process headers payload
			
			var buffer bytes.Buffer  // Use bytes.Buffer to efficiently build the output string
			// Iterate through the slice of maps
			for i, record := range analyzedResult {
				name := record["name"]
				value := record["value"]
				if i > 0 {
					buffer.WriteString(" ")  // Add space before the next item except for the first one
				}
				// Format and append to the buffer
				buffer.WriteString(fmt.Sprintf("(%s, %s)", name, value))
			}
			fmt.Println("Header data: [", buffer.String(), "]")

        case 0x2:
            fmt.Println("PRIORITY frame")

        case 0x3:
            fmt.Println("RST_STREAM frame")

        case 0x4:
            fmt.Println("SETTINGS frame")

        case 0x5:
            fmt.Println("PUSH_PROMISE frame")

        case 0x6:
            fmt.Println("PING frame")

        case 0x7:
            fmt.Println("GOAWAY frame")

        case 0x8:
            fmt.Println("WINDOW_UPDATE frame")

        case 0x9:
            fmt.Println("CONTINUATION frame")

        default:
            fmt.Println("Unknown frame type")
        }

        frameIndex += 9 + int(frameLength)
    }
}

func splitHex(hexInput string) []string {
    return strings.Fields(hexInput) // Splits the string by spaces
}

func equalSlice(a, b []string) bool {
    if len(a) != len(b) {
        return false
    }
    for i := range a {
        if a[i] != b[i] {
            return false
        }
    }
    return true
}

func concatHex(hexSlice []string) string {
    var sb strings.Builder
    for _, hex := range hexSlice {
        sb.WriteString(hex)
    }
    return sb.String()
}