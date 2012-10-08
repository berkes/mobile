# About

Mobile leverages Varnish proxy for switching to a mobile theme. 

In contrary to existing modules for Drupal that give mobile support,
this module has:
* High performance.
* A simple and predictable switching logic.
* No in-database settings, simple to deploy.

Because of this, we made some choices, which means it does not have:
* Mobile detection. Varnish takes care of that.
* User-preferences: users cannot change the theme with e.g. a link.

The only options you need to set are:
* What theme is the mobile theme, what is the table theme and what is the desktop theme.
* Whether or not the module should redirect users to and from
  m.example.com and www.example.com (or whether your proxy takes care of
  that)

# Installation
Install the module just like all other modules. This module has no admin
or configuration screens.

Add the settings to your settings.php:

    $conf["mobile"]["desktop"] = array(
      "theme" => "my_desktop_theme",
      "url"   => "www.example.com",
      "redirect" => true
    );
    $conf["mobile"]["tablet"] = array(
      "theme" => "my_touch_theme",
      "url"   => "touch.example.com",
      "redirect" => false
    );
    $conf["mobile"]["mobile"] = array(
      "theme" => "my_mobile_theme",
      "url"   => "m.example.com",
      "redirect" => true
    );

Settings are an array of arrays with arrays (Hey, it's Drupal after all!). The first
key, is the namespace for this module:

   $conf["mobile"] // Everything under here is part of settings for this module

The second ring is an array which describes the devices. This will react
to the `X-devise`-headers. E.g. `$conf["mobile"]["foo"]` will react to `X-devise = 'foo'`.

The third ring is the actual per-devise setting. It has three keys:
"theme", "url" and "redirect". 

* *theme*  a string representing the system name for the theme. Can be
  looked up in e.g. the `system` table in Drupal with 
    `select * from system where type = "theme"`
* *url* the URL, without protocol and path, but with optional port
  (defaults to :80). Mobile module will trigger on this item too.
* *redirect* a boolean defining whether or not a redirect should be
  issued.

# Theme switching and redirection logic

## Situation: redirection is off, varnish-headers are off:
* User visits "www.example.com" the theme is set to "my_desktop_theme".
* User visits "touch.example.com" theme is set to "my_touch_theme".
* User visits "m.example.com" theme is set to "my_mobile_theme".

Nothing else is done, issued or invoked.

## Situation: redirection is off, varnish-headers are on:
* User visits "www.example.com" with a desktop-browser, theme is set to "my_desktop_theme".
* User visits "touch.example.com" with a tablet, theme is set to "my_touch_theme".
* User visits "m.example.com" with a mobile devise, theme is set to "my_mobile_theme".
* User visits "www.example.com" with a tablet, theme is set to "my_touch_theme".
* User visits "www.example.com" with a mobile devise, theme is set to "my_mobile_theme".
* User visits "touch.example.com" with a desktop-browser, theme is set to "my_touch_theme".
* etceteras.

## Situation: redirection is on, varnish-headers are off:

Exactly the same as with redirection off. Thss situation is unwanted and considered
"broken"; redirects are issued according to varnish headers, if they are
not sent, redirection will not take place.

## Situation: redirection is on, varnish-headers are on:

* User visits "www.example.com" with a desktop-browser, theme is set to "my_desktop_theme". No redirect.
* User visits "touch.example.com" with a tablet, theme is set to "my_touch_theme". No redirect.
* User visits "m.example.com" with a mobile devise, theme is set to "my_mobile_theme". No redirect.
* User visits "www.example.com" with a tablet, user is redirected to "touch.example.com", then theme is set to "my_touch_theme".
* User visits "www.example.com" with a mobile devise, user is redirected to "m.example.com", then theme is set to "my_mobile_theme".
* User visits "touch.example.com" with a desktop-browser, user is redirected to "www.example.com", then theme is set to "my_touch_theme".
* etceteras.

# Author
Bèr Kessels <ber@webschuur.com>
For [Dutch Open Projects](http://dop.nu/)

# Requirements
Varnish proxy, or another proxy that can send an `X-Device` header.

# TODOs
Redirection higher up in the Drupal bootstrap. Right now 90% or more of the
application is loaded and executed before a redirect; causing massive,
unnessecary overhead.
