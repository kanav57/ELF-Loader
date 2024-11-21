#invoke make inside following directories and in this order: loader, launch, fib
all:
	make -C ./loader
	make -C ./launcher
	make -C ./test

#move the lib_simpleloader.so and launch binaries inside bin directory
move:
	mv ./loader/lib_simpleloader.so ./bin
	mv ./launcher/launch ./bin
	mv ./test/fib ./bin
#Provide the command for cleanup
clean:
	@rm -f ./bin/*