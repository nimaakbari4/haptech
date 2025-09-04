SHELL := /bin/bash

.PHONY: bootstrap
bootstrap:
	@chmod +x scripts/bootstrap.sh scripts/bootstrap-macos.sh scripts/bootstrap-linux.sh || true
	@./scripts/bootstrap.sh $(ARGS)

