CC = g++
CXX_FLAGS = -std=c++11 -w 
# gdbµ÷ÊÔÑ¡Ïî -g -rdynamic

BIN = unix-fs

$(BIN): main.o Buffer.o BufferManager.o DeviceDriver.o File.o FileManager.o FileSystem.o INode.o OpenFileManager.o User.o Utility.o
	$(CC) -o $@ $^
main.o: main.cpp
	$(CC) $(CXX_FLAGS) -c -o $@ $<
Buffer.o: Buffer.cpp
	$(CC) $(CXX_FLAGS) -c -o $@ $<
BufferManager.o: BufferManager.cpp
	$(CC) $(CXX_FLAGS) -c -o $@ $<
DeviceDriver.o: DeviceDriver.cpp
	$(CC) $(CXX_FLAGS) -c -o $@ $<
File.o: File.cpp
	$(CC) $(CXX_FLAGS) -c -o $@ $<
FileManager.o: FileManager.cpp
	$(CC) $(CXX_FLAGS) -c -o $@ $<
FileSystem.o: FileSystem.cpp
	$(CC) $(CXX_FLAGS) -c -o $@ $<
INode.o: INode.cpp
	$(CC) $(CXX_FLAGS) -c -o $@ $<
OpenFileManager.o: OpenFileManager.cpp
	$(CC) $(CXX_FLAGS) -c -o $@ $<
User.o: User.cpp
	$(CC) $(CXX_FLAGS) -c -o $@ $<
Utility.o: Utility.cpp
	$(CC) $(CXX_FLAGS) -c -o $@ $<
	
.phony:
clean:
	rm -f *.o $(BIN)
	rm -f *.img