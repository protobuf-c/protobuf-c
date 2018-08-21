
all:
	gcc -o list_people addressbook.pb-c.c list_people.c -I$(HOME)/proto_c/include -L/$(HOME)/proto_c/lib -lprotobuf-c
	g++ -o add_person addressbook.pb.cc add_person.cc `pkg-config --cflags --libs protobuf`

