SUBDIRS = daemon dsp/test

all: subdirs

subdirs:
	for dir in $(SUBDIRS); do\
	  $(MAKE) -C $$dir || break; \
	done

clean:
	for dir in $(SUBDIRS); do\
	  $(MAKE) -C $$dir clean; \
	done

rebuild: clean subdirs

