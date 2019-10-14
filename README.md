# interception-k2k
Branch of [interception-k2k](https://github.com/zsugabubus/interception-k2k) with hardcoded common configuration to remap `caps-lock` to `ctrl` when held and `escape` when tapped.

## Configuration

Modify `/etc/udevmon.yaml` to contain:
```yaml
- JOB: "intercept -g $DEVNODE | /opt/interception/simple_caps2esc | uinput -d $DEVNODE"
  DEVICE:
      EVENTS:
            EV_KEY: [KEY_CAPSLOCK]
```

## Installation

```sh
git clone https://github.com/milesfrain/interception-k2k
cd interception-k2k
make
make install
```
