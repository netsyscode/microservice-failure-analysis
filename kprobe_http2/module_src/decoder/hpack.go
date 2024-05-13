package decoder

import (
    "encoding/hex"
    // "fmt"
)

var hpackTable = map[int][2]string{
    1: {":authority", ""},
    2: {":method", "GET"},
    3: {":method", "POST"},
    4: {":path", "/"},
    5: {":path", "/index.html"},
    6: {":scheme", "http"},
    7: {":scheme", "https"},
    8: {":status", "200"},
    9: {":status", "204"},
    10: {":status", "206"},
    11: {":status", "304"},
    12: {":status", "400"},
    13: {":status", "404"},
    14: {":status", "500"},
    15: {"accept-charset", ""},
    16: {"accept-encoding", "gzip, deflate"},
    17: {"accept-language", ""},
    18: {"accept-ranges", ""},
    19: {"accept", ""},
    20: {"access-control-allow-origin", ""},
    21: {"age", ""},
    22: {"allow", ""},
    23: {"authorization", ""},
    24: {"cache-control", ""},
    25: {"content-disposition", ""},
    26: {"content-encoding", ""},
    27: {"content-language", ""},
    28: {"content-length", ""},
    29: {"content-location", ""},
    30: {"content-range", ""},
    31: {"content-type", ""},
    32: {"cookie", ""},
    33: {"date", ""},
    34: {"etag", ""},
    35: {"expect", ""},
    36: {"expires", ""},
    37: {"from", ""},
    38: {"host", ""},
    39: {"if-match", ""},
    40: {"if-modified-since", ""},
    41: {"if-none-match", ""},
    42: {"if-range", ""},
    43: {"if-unmodified-since", ""},
    44: {"last-modified", ""},
    45: {"link", ""},
    46: {"location", ""},
    47: {"max-forwards", ""},
    48: {"proxy-authenticate", ""},
    49: {"proxy-authorization", ""},
    50: {"range", ""},
    51: {"referer", ""},
    52: {"refresh", ""},
    53: {"retry-after", ""},
    54: {"server", ""},
    55: {"set-cookie", ""},
    56: {"strict-transport-security", ""},
    57: {"transfer-encoding", ""},
    58: {"user-agent", ""},
    59: {"vary", ""},
    60: {"via", ""},
    61: {"www-authenticate", ""},
}

// Function to get a string value from a header payload
func getStringValue(headersPayload []byte, index int) (string, int) {
    valueLength := int(headersPayload[index]) & 0x7F
    stringData := headersPayload[index+1 : index+1+valueLength]
    if headersPayload[index]&0x80 == 0x80 {
        // Huffman decode needed
        encodedHex := hex.EncodeToString(stringData)
        decodedString := huffmanDecode(encodedHex) // Define this function based on your Huffman decoding needs
        return decodedString, index + valueLength + 1
    } else {
        // Direct ASCII conversion
        return string(stringData), index + valueLength + 1
    }
}

// Analyzing headers payload
func analyzeHeadersPayload(headersPayload []byte) []map[string]string {
    var headers []map[string]string
    index := 0
    for index < len(headersPayload) {
        header := make(map[string]string)
        if headersPayload[index]&0x80 == 0x80 { // Indexed Header Field Representation
            headerIndex := int(headersPayload[index]) & 0x7F
            header["name"], header["value"] = hpackTable[headerIndex][0], hpackTable[headerIndex][1]
            index++
        } else if headersPayload[index]&0x40 == 0x40 { // Literal Header Field with Incremental Indexing
            headerIndex := int(headersPayload[index]) & 0x3F
            if headerIndex > 0 {
                header["name"] = hpackTable[headerIndex][0]
                header["value"], index = getStringValue(headersPayload, index+1)
				hpackTable[len(hpackTable)+1] = [2]string{header["name"], header["value"]}
            } else {
                header["name"], index = getStringValue(headersPayload, index+1)
                header["value"], index = getStringValue(headersPayload, index)
				hpackTable[len(hpackTable)+1] = [2]string{header["name"], header["value"]}
            }
        } else if headersPayload[index]&0xF0 == 0x00 { // Literal Header Field without Indexing
            headerIndex := int(headersPayload[index]) & 0x0F
			if headerIndex > 0 {
                header["name"] = hpackTable[headerIndex][0]
                header["value"], index = getStringValue(headersPayload, index+1)
            } else {
                header["name"], index = getStringValue(headersPayload, index+1)
                header["value"], index = getStringValue(headersPayload, index)
            }
            // Additional handling based on your protocol needs
        }
        headers = append(headers, header)
    }
    return headers
}