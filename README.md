Useful component to debug cheap radios that produce too much noise and overwhelm remote receiver. 

Uses a gpio isr instead of RMT. Looks for patterns and only outputs data that isnt random. Can adjust the max capture length, shortest pulse length and idle limit. 

If you can't get anything out of remote receiver you can try this. 

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