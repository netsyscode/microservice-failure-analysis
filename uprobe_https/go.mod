module my_modules

go 1.22

replace my_modules => ./

require (
	github.com/alexflint/go-arg v1.4.3
	github.com/iovisor/gobpf v0.2.0
	golang.org/x/sys v0.0.0-20220227234510-4e6760a101f9
)

require (
	github.com/alexflint/go-scalar v1.1.0 // indirect
	gopkg.in/yaml.v3 v3.0.0-20210107192922-496545a6307b // indirect
)
