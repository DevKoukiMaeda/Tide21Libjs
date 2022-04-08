Tide21Lib: tide.cpp Tide21Lib.cpp Tide21Lib.h
	emcc -s EXPORT_NAME="'TideMod'" -s MODULARIZE=1 -s "EXPORTED_RUNTIME_METHODS=['ccall','cwrap','stringToUTF8','UTF8ToString','allocate','intArrayFromString','ALLOC_NORMAL']" -s WASM=1 -s NO_EXIT_RUNTIME=1 -s EXPORTED_FUNCTIONS=_cr_tide,_calctide --bind -o bin/Tide21Lib.js tide21lib.cpp tide.cpp   -lm --preload-file "./DT"
