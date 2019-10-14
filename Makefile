CFLAGS+=-std=c99 -O3 -Wall -Wextra -Werror -Wno-type-limits
TIMEOUT?=30

OUT_DIR:=out
INSTALL_DIR:=/opt/interception

TARGET:=$(OUT_DIR)/simple_caps2esc

.PHONY: all
all: $(TARGET)

$(TARGET): k2k.c | $(OUT_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OUT_DIR):
	mkdir $@

.PHONY: clean
clean:
	rm -rf $(OUT_DIR)

.PHONY: install
install:
	# If you have run `make test` then do not forget to run `make clean` after. Otherwise you may install with debug logs on.
	install -D --strip -t /opt/interception $(TARGET)

.PHONY: test
test:
	CFLAGS=-DVERBOSE make
	make install
	timeout $(TIMEOUT) udevmon -c /etc/udevmon.yaml
