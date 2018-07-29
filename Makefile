CXX = g++
CXXFLAGS = -Wall -std=c++11
LDFLAGS = -static -lstdc++ -lgcc -lwinpthread
WNDFLAGS = -Wl,-Bdynamic -lwndhook
EXE = wintile.exe

.PHONY: all
all: $(EXE)

wintile.exe: wintile.o wndhook.dll
	$(CXX) $(CXXFLAGS) $< -o $@ -mwindows $(LDFLAGS) -L./ $(WNDFLAGS)

wintile.o: wintile.cc wintile.h wndhookdll.h
	$(CXX) $(CXXFLAGS) -c $<

wndhook.dll: wndhookdll.o
	$(CXX) $(CXXFLAGS) -shared $< -o $@ $(LDFLAGS)

wndhookdll.o: wndhookdll.cc wndhookdll.h
	$(CXX) $(CXXFLAGS) -c $<

debug: wintile.o wndhook.dll
	$(CXX) $(CXXFLAGS) -S -g wintile.cc wndhookdll.cc
	$(CXX) $(CXXFLAGS) -c -g wintile.cc wndhookdll.cc
	$(CXX) $(CXXFLAGS) -shared wndhookdll.o -o wndhook.dll
	$(CXX) $(CXXFLAGS) -L./ -lwndhook wintile.o -o $(EXE) -mwindows

release: wintile.o wndhook.dll
	$(CXX) $(CXXFLAGS) -shared -s -DNDEBUG wndhookdll.o -o wndhook.dll
	$(CXX) $(CXXFLAGS) -L./ -lwndhook -s -DNDEBUG wintile.cc -o $(EXE) -mwindows

.PHONY: clean
clean:
	rm *.o *.s

