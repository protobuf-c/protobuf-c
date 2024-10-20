ARG VERSION="1.5"
# Assume this is run from capstone/debian directory
# Run in the root of the repo
# docker build -f ./debian/Dockerfile -t packager .
FROM debian:buster-slim

# Install necessary tools for packaging
RUN apt-get -qq update && \
    DEBIAN_FRONTEND=noninteractive apt-get -qq install -y \
        fakeroot dpkg-dev dos2unix pkg-config curl unzip protobuf-compiler libprotobuf-dev libprotoc-dev autoconf libtool

# protobuf-c requires a C compiler, a C++ compiler, protobuf, and pkg-config (included) to be installed.
# Install dependencies protobuf, put the zip into /usr/local 
RUN curl -LJO https://github.com/protocolbuffers/protobuf/releases/download/v24.0/protoc-24.0-linux-x86_64.zip
RUN unzip protoc-24.0-linux-x86_64.zip -d /usr/local

# Then install capstone from source
RUN mkdir /protobuf-c

# Copy your project files into the container
COPY . /protobuf-c

# Ensure autogen.sh is executable, run it and configure
RUN dos2unix /protobuf-c/autogen.sh
RUN chmod +x /protobuf-c/autogen.sh

# Run the make.sh script and install
WORKDIR /protobuf-c/
RUN ./autogen.sh
RUN ./configure
RUN make

# List files before make install
# RUN find / -type f > /before-install.txt

# Run make install
RUN make install

# List files after make install
# RUN find / -type f > /after-install.txt

# Create necessary directories in /package-root
RUN mkdir -p /package-root/usr/local/include/protobuf-c
RUN mkdir -p /package-root/usr/local/lib/pkgconfig
RUN mkdir -p /package-root/usr/local/lib/libprotobuf
RUN mkdir -p /package-root/usr/local/bin/

# Copy the files created from 'make install' into /package-root
RUN cp -r /usr/local/include/protobuf-c/* /package-root/usr/local/include/protobuf-c/
RUN cp -r /usr/local/lib/pkgconfig/* /package-root/usr/local/lib/pkgconfig/
RUN cp -r /usr/local/lib/libprotobuf-c.la /package-root/usr/local/lib/
RUN cp -r /usr/local/lib/libprotobuf-c.so.1.0.0 /package-root/usr/local/lib/
RUN cp -r /usr/local/lib/libprotobuf-c.a /package-root/usr/local/lib/
RUN cp -r /usr/local/bin/protoc-gen-c /package-root/usr/local/bin/

# Create DEBIAN directory and control file
COPY ./debian/control /package-root/DEBIAN/control

# Update control file with the correct version
ARG VERSION
RUN sed -i "s/^Version:.*/Version: ${VERSION}/" /package-root/DEBIAN/control

# Build the package
RUN fakeroot dpkg-deb --build /package-root /protobuf-c.deb

# The user can now extract the .deb file from the container with something like
# docker run --rm -v $(pwd):/out packager bash -c "cp /protobuf-c.deb /out"