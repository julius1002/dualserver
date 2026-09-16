# Hello world example with a greeting

## How to build

1. first build the static library in the project root

    make static

2. then go back to this directory

    cd examples/json

3. get the json dependency:
    wget https://github.com/DaveGamble/cJSON/archive/refs/tags/v1.7.19.zip && unzip v1.7.19.zip

4. build json dependency:
    cd cJSON-1.7.19 && make

4. then build the example

    cd .. && make
