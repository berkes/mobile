require "./lib"

["mobile", "tablet", "desktop"].each do |profile|
  mt = MobileTestAgent.new("http://dom.audrey")
  mt.x_devise = profile
  response = mt.get
  p "  profile: #{profile}"
  p "issue GET: #{mt.url}"
  p "      GOT: #{response.uri.to_s}"
end
