Authenticator
=============

This is Authenticator for Pebble, a self-contained watch app for Pebble SDK 2.0,
generating multiple Time-based One-Time Passwords, much like Google Authenticator.
It uses the phone for a configuration UI and long-term configuration storage across
upgrades, but the watch can continue to generate passwords without a connection to
the phone after the initial configuration is set.

To use, install the pbw, then go to "Watch Apps" in the Pebble app and tap the
configuration icon next to Authenticator to enter your account/secret information.

The watch retrieves UTC offset (time zone) information from the phone during
configuration, as the Pebble does not natively support time zones.

If the app is started after not having been used for at least an hour and there is
a connection to the phone, it will ask for updated time zone information.  In addition,
pressing and holding the select button will tell the watch to immediately refresh the
configuration and timezone information.

NOTE that this assumes that your phone's time and time zone are set correctly!  This
should not be an issue if you are getting network time, but if you are travelling
somewhere without network service and you update your phone's clock, you must also
inform it of the new timezone or it will generate incorrect passwords.


Credits
-------

pokey9000: Initial implementation, called 'twostep'  
IEF: Support for multiple tokens and labels, new user interface  
rigel314: optimizations and changing timezone support  
Danielle Church: SDK 2.0 support, JS configuration, on-watch storage  
