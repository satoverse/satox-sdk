module github.com/satoverse/satox-sdk

go 1.21

require (
	github.com/google/uuid v1.4.0
	github.com/stretchr/testify v1.8.4
)

require (
	github.com/davecgh/go-spew v1.1.1 // indirect
	github.com/pmezard/go-difflib v1.0.0 // indirect
	gopkg.in/yaml.v3 v3.0.1 // indirect
)

// +build cgo

replace satox => ./satox
