SUBDIRS = $(shell find . -mindepth 1 -maxdepth 1 -type d)

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean BUILD_DIR=$(BUILD_DIR); \
	done
