Useful component for cheap radios that produce too much noise and overwhelm remote receiver. 

Uses a gpio isr instead of RMT. If you cant get anything out of remote receiver you can try this. 

Meant to be used more for debugging but could be used permanently.

    external_components:
      - source:
          type: git
          url: https://github.com/swoboda1337/remote_receiver_isr
          ref: main
        refresh: 5min

    remote_receiver_isr:
      pin: GPIO23
      idle: 5000us
      shortest: 100us
      capture: 156
      on_raw:
        then:
          - lambda: |-
              remote_base::RemoteReceiveData data(x, 40, remote_base::TOLERANCE_MODE_PERCENTAGE);
              remote_base::RawDumper dumper;
              dumper.dump(data);
              
              remote_base::KeeloqProtocol keeloq;
              auto ret = keeloq.decode(data);
              if (ret.has_value()) keeloq.dump(ret.value());