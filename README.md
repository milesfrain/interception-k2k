# interception-k2k
Fork of [interception-k2k](https://github.com/zsugabubus/interception-k2k) with common configuration to remap `caps-lock` to `ctrl` when held and `escape` when tapped.

## Configuration

This config can be found in `example/interception-pipe0/tap-rules.h.in`:
```
{ .base_key = KEY_CAPSLOCK, .tap_key = KEY_ESC, .repeat_key = KEY_LEFTCTRL, .hold_key = KEY_LEFTCTRL },
```

Modify `/etc/udevmon.yaml` to contain:
```yaml
- JOB: "intercept -g $DEVNODE | /opt/interception/interception-pipe0 | uinput -d $DEVNODE"
  DEVICE:
    EVENTS:
      EV_KEY: [KEY_CAPSLOCK]
```

## Installation

```sh
git clone https://github.com/milesfrain/interception-k2k
cd interception-k2k
make CONFIG_DIR=example
make CONFIG_DIR=example install
```
