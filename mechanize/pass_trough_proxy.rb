require "./lib"

["iPhone", "Windows IE 7", "Mac Safari"].each do |a|
  mt = MobileTestAgent.new("http://dom.audrey")
  mt.user_agent_alias = a
  response = mt.get
  p "  profile: #{a}"
  p "issue GET: #{mt.url}"
  p "      GOT: #{response.uri.to_s}"
end
