.PHONY: all clean threads process sync ui

all: threads process sync ui

threads:
	$(MAKE) -C threads

process:
	$(MAKE) -C process

sync:
	$(MAKE) -C synchronization

ui:
	$(MAKE) -C ui

clean:
	$(MAKE) -C threads clean
	$(MAKE) -C process clean
	$(MAKE) -C synchronization clean
	$(MAKE) -C ui clean 