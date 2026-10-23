# Build stage: compile in Release mode and run the test suite.
# The image build fails if any test fails.
FROM ubuntu:24.04 AS build

RUN apt-get update \
    && apt-get install -y --no-install-recommends cmake g++ make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel \
    && ctest --test-dir build --output-on-failure

# Runtime stage: only the demo executable.
FROM ubuntu:24.04

COPY --from=build /src/build/options_pricer /usr/local/bin/options_pricer

CMD ["options_pricer"]
