all: graph client tracker timeserv

graph:
	make -f graph.make --no-print-directory
	@echo "-----------------------------------"
client:
	make -f client.make --no-print-directory
	@echo "-----------------------------------"
tracker:
	make -f tracker.make --no-print-directory
	@echo "-----------------------------------"
timeserv:
	make -f timeserv.make --no-print-directory

clean:
	rm -f *.o
	rm -f netg netc nett netw
	rm -f *~
	rm -f \#*\#
