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
* The urls that belong to a profile, e.g. m.example.com for mobile and www.example.com for desktop.

Note that one could optionally set the proxy to do the redirecting. That
would take some load from Drupal, but it would spread out the
configuration over the settings in Drupal and the varnish conf. If you
want to benefit from that, you'll need to hack out the `hook_boot`,
elswise the code in Drupal will be loaded nonetheless.

# Installation
Installation and configuration requires actions in Drupal and in
Varnish.

# Drupal
Install the module just like all other modules. This module has no admin
or configuration screens.

Add the settings to your settings.php:

    $conf["mobile"]["desktop"] = array(
      "theme" => "my_desktop_theme",
      "url"   => "http://www.example.com",
    );
    $conf["mobile"]["tablet"] = array(
      "theme" => "my_touch_theme",
      "url"   => "http://touch.example.com",
    );
    $conf["mobile"]["mobile"] = array(
      "theme" => "my_mobile_theme",
      "url"   => "http://m.example.com",
    );

Settings are an array of arrays with arrays (Hey, it's Drupal after all!). The first
key, is the namespace for this module:

    $conf["mobile"] // Everything under here is part of settings for this module

The second ring is an array which describes the devices. This will react
to the `X-Devise`-headers.

    $conf["mobile"]["foo"] // will react to `X-Devise = 'foo'`.

The third ring is the actual per-devise setting. It has three keys:
"theme" and "url".

* *theme*  a string representing the system name for the theme. Can be
  looked up in e.g. the `system` table in Drupal with 
    `select * from system where type = "theme"`
* *url* the URL, without protocol and path, but with optional port
  (defaults to :80). Mobile module will trigger on this item too.

## Development and testing.

Testing can be done with [TamperData](https://addons.mozilla.org/en-US/firefox/addon/tamper-data/) to set the `X-Devise`-header from within Firefox.

A better option is to use the provided Mechanize scripts; they forge the
headers. These scripts are hackish, have hardcoded values and need to be
opened up and edited before you can use them.

    $ cd ./mechanize
    $ gem install "mechanize"

    # Runs a script that manually sets headers.
    $ ruby ./self_set_header.rb
    # Runs a script that sets the user agent; Varnish should pick it up.
    $ ruby ./pass_trough_proxy.rb

## Varnish

You will need to add simple mobile-agent detection to the proxy. It is
advised to keep this as simple as possible; else false-positives
(desktop-browsers wrongfully detected as a mobile browser) might make
things harder.Below is taken from [Tom Deryckere's post](http://www.eldeto.com/content/mobile-device-detection-varnish-0)
and adapted for this module. Use this as example, please don't copy-paste it blindly into your Varnish. Things
will break and you will be the one to blame. At least try to understand
the logic.

    # Default to thinking it's a PC
    set req.http.X-Device = "desktop";

    if (req.http.User-Agent ~ "iPad" ) {
      # It says its a iPad - so let's give them the tablet-site
      set req.http.X-Device = "tablet";
    }
    elsif (req.http.User-Agent ~ "iP(hone|od)" || req.http.User-Agent ~ "Android" ) {
      # It says its a iPhone, iPod or Android - so let's give them the touch-site..
      set req.http.X-Device = "mobile";
    }
    elsif (req.http.User-Agent ~ "SymbianOS" || req.http.User-Agent ~ "^BlackBerry" || req.http.User-Agent ~ "^SonyEricsson" || req.http.User-Agent ~ "^Nokia" || req.http.User-Agent ~ "^SAMSUNG" || req.http.User-Agent ~ "^LG") {
      # Some other sort of mobile
      set req.http.X-Device = "mobile";
    }

Add this to your Varnish configuration, e.g. in
`/etc/varnish/conf.d/devise-detect.vcl`

# Theme switching and redirection logic
The switching and redirection logic is kept as simple and predictable as
possible. This to avoid any edge-cases and weird bugs.

* Varnish-header `X-Devise` dictates what profile _should_ be activated;
  regardless of the url the user is on.
* The url the user _is_ on, is compared to the url the user _should be_
  on, according to the profile given with the `X-Devise`-header.
  * If they do not match, the user is redirected.
  * If they match, the theme for the profile will be activated.

This means:

* One URL will always serve the same theme. This makes caching a lot
  more predictable and simpler.
* Users will always be redirected to the URL they should belong on,
  based on the devise-detection in the proxy.

## Situation: varnish-headers are off:
* User visits "www.example.com" the theme is set to "my_desktop_theme".
* User visits "touch.example.com" theme is set to "my_touch_theme".
* User visits "m.example.com" theme is set to "my_mobile_theme".

Nothing else is done, issued or invoked.

## Situation: varnish-headers are on:
* User visits "www.example.com" with a desktop-browser, theme is set to "my_desktop_theme". No redirect.
* User visits "touch.example.com" with a tablet, theme is set to "my_touch_theme". No redirect.
* User visits "m.example.com" with a mobile devise, theme is set to "my_mobile_theme". No redirect.
* User visits "www.example.com" with a tablet, user is redirected to "touch.example.com", then theme is set to "my_touch_theme".
* User visits "www.example.com" with a mobile devise, user is redirected to "m.example.com", then theme is set to "my_mobile_theme".
* User visits "touch.example.com" with a desktop-browser, user is redirected to "www.example.com", then theme is set to "my_touch_theme".
* etceteras.

# Different settings for sites
Many modules that handle "mobile" do much more then handling mobile.
They, for example, offer options to have alternative frontpages based on
the device. This module does nothing of such. It delegates this to
Drupal, which is more sturdy, predictable and performant.

Say, you want a different title and frontpage for tables and mobile,
here is how to achieve that.

## Given
A setup with mobile where
* www.example.com is the "desktop"
* touch.example.com is the "tablet/touch" version.
* m.example.com is the "mobile" version.

## When
This all works, and user is correctly redirected and gets correct
theme based on the Varnish Headers.

## Then
Add three so-called "multisite environments":

* Create three directories; from Drupal's docroot:
  * `sites/www.example.com`
  * `sites/touch.example.com`
  * `sites/m.example.com`
* Add cleaned and empty settings.php files in there, then add a little
  content; for example `sites/touch.example.com/settings.php`:

    <?php
    require "../default/settings.php" # This file extends the default
settings.php
    $conf["site_frontpage"] = "home-touch";
    $conf["site_name"] = "Example.com, now 100% tablet-compatible!"

This sets the url for the homepage to "home-touch", where you can serve
alternative content for the "touch" devices. It also changes the
_site\_name_ which appears in the title of the site.

You can change any settings in these settings.php-files, including
memory-limits (e.g. touch-devises and mobile might never need logged-in
users and can run with much slimmer PHP-threads), database-credentials
(e.g. to use a slave database-server for mobile) and so on. 

## Thread-safety
This setup is also reasonably thread-safe. Not much (so, actually, it is simply not
threadsafe at all) but a lot more threadsafe then most other mobile
environments who attempt to set these global(!) persistent(!) variables
in-thread. Affecting all other running threads, regardless of their
"mobile-status". In any case: never, ever set these variables in the
hook_init and its friends. Ever.

# Author
Bèr Kessels <ber@webschuur.com>
For [Dutch Open Projects](http://dop.nu/)

# Requirements
Varnish proxy, or another proxy that can send an `X-Device` header.

# TODOs
Since the protocol is hardcoded into the url-setting, you cannot
dynamically switch to and from https. Dynamically redirecting to and
from https might also conflict with other modules made for this purpose.
We should make a detection of http versus https and redirect the user to
the same protocol he or she is already on.

Drush breaks; mobile should be
* Wrapped in a condition so it does not redirect etc. in case of Drush
  and CLI
* aware of Drush commandline-options to set Varnish-headers.

# Known issues
Drupal [does not allow wap:// or mobile://](http://api.drupal.org/api/drupal/includes!common.inc/function/drupal_strip_dangerous_protocols/7) protocol, only a subset of undocumented protocols. You cannot redirect to these kind of URLS.

This module will not work together with modules that do redirections
(such as mobile tools, domain-access, securepages), nor with themes that handle mobile detection (and redirects) themselves. If you run into conflicts, just remove the other module :).
