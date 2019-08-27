all:
	$(MAKE) -C server
	$(MAKE) -C client

help:
	@echo
	@echo "===============A common Makefile for c programs=============="
	@echo "Copyright (C) 2019/8/27 liankunyu "
	@echo "The following targets are support:"
	@echo
	@echo " all              - (==make) compile and link"
	@echo " clean            - clean target"
	@echo " help             - print help information"
	@echo
	@echo "========================= Version1.0 ======================="

clean:
	$(MAKE) -C server clean
	$(MAKE) -C client clean

.PHONY: all help clean
