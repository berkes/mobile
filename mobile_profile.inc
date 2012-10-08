<?php

/**
 * Class MobileProfile 
 * @author Bèr Kessels <ber@webschuur.com> for Dutch Open Projects
 * <http://dop.nu>
 *
 * @code
 * $tablet_profile = new MobileProfile("tablet");
 * $tablet_profile->activate();
 * @endcode
 */
class MobileProfile {
  private $profiles = array();
  public  $active = FALSE;
  public  $theme = "";
  public  $url   = "";
  public  $redirect = FALSE;

  /**
   * new MobileProfile
   * @param String $profile_name name of the mobile profile. 
   *        symbol, "foo" as used as key in $conf["mobile"]["foo"].
   */
  public function __construct($profile_name) {
    $profiles = variable_get("mobile", array());
    if (empty($profiles)) {
      throw new Exception(t("no mobile profiles configured"));
    }
    elseif (!array_key_exists($profile_name, $profiles)) {
      throw new Exception(t("%profile_name not configured", array("%profile_name" => $profile_name )));
    }
    else {
      $this->profiles = $profiles;

      $profile = $profiles[$profile_name];
      $this->theme    = $profile["theme"];
      $this->url      = $profile["url"];
    }
  }

  /**
   * Select and prepare a new $this by the url.
   */
  static public function select_by_url() {
    $profiles = variable_get("mobile", array());
    //@TODO: DRY from is_on_url(). But since this is a static call, 
    //       it is a little harder to make general.
    foreach($profiles as $profile_name => $profile) {
      $scheme = empty($_SERVER["HTTPS"]) ? "http" : "https" ;
      $host   = $_SERVER["HTTP_HOST"];
      $this_url = parse_url($profile["url"]);
      if ($scheme == $this_url["scheme"] && $host == $this_url["host"]) {
        $class_name = get_class();
        return new $class_name($profile_name);
      }
    }
    return NULL;
  }

  // Activate this profile and fire the theme switcher and the
  // redirector.
  public function activate() {
    $this->active = TRUE;
    $this->redirect();
  }

  // Detect whether we are on the URL as provided in this profile.
  private function is_on_url() {
    $scheme = empty($_SERVER["HTTPS"]) ? "http" : "https" ;
    $host   = $_SERVER["HTTP_HOST"];
    $this_url = parse_url($this->url);
    return ($scheme == $this_url["scheme"] && $host == $this_url["host"]);
  }

  // Detect whether $this theme is active.
  private function theme_is_active() {
    global $conf;
    return ($conf["theme_default"] == $this->theme);
  }

  // Builds the full URL for a redirect: includes any queries, paths and
  // whatnots.
  private function full_url() {
    $request_uri = request_uri();
    return "{$this->url}{$request_uri}";
  }

  // Invoke a Drupal-goto. Detects whether it is needed at all.
  private function redirect() {
    if (!$this->is_on_url()) {
      header('Location: ' . $this->full_url(), TRUE, 301);
      drupal_exit($this->full_url());
    }
  }
}
