Stagg EKG+ ESPHome
==================

I hacked together a simple proxy to integrate the Stagg EKG+ with home assistant.

I expose the kettle as a `climate` device that supports on/off and setting the target temperature. The integration also shows the state (is the heating element on) as well as the current temperature.

![HA View](images/ha_view.png)

Special thanks to: https://github.com/tlyakhov/fellow-stagg-ekg-plus as I used his kettle object as the starting point for this project.



