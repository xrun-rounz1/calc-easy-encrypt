FROM debian:12.10-slim AS build
RUN apt-get update -qq  && \
    apt-get -qq install --no-install-recommends -y apt-utils g++ libc-dev sqlite3 libsqlite3-dev > /dev/null  && \
    apt-get -qq clean && \
    apt-get -qq autoclean && \
    rm -rf /var/lib/apt/lists/* && \
    rm -rf /usr/share/doc/* 

RUN mkdir /build

ADD *.cpp /build/
ADD data.sql /build/

WORKDIR /build

RUN g++ -o easy-decrypt decrypt.cpp -lsqlite3
RUN sqlite3 video.db < data.sql


FROM gcr.io/distroless/cc-debian12:nonroot

COPY --from=build /build/easy-decrypt /app/
COPY --from=build /build/video.db /app/
COPY --from=build /lib/x86_64-linux-gnu/libsqlite3.so.0 /lib/

WORKDIR /app
ENTRYPOINT ["/app/easy-decrypt"]
