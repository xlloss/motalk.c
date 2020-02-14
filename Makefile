motalk: at_cmd.o
	${CC} at_cmd.o -o at_cmd


motalk.o: motalk.c
	${CC} -c at_cmd.c

clean:
	rm *.o at_cmd
