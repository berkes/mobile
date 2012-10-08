require 'rubygems'
require 'mechanize'
require 'pp'

class MobileTestAgent
  attr_accessor :url
  def initialize url
    @url = url
    @agent = Mechanize.new()
  end

  def x_devise=x_devise
    @agent.request_headers = {"X-Devise" => x_devise }
  end

  # see Mechanize::user_agent_alias
  def user_agent_alias=user_agent_alias
    @agent.user_agent_alias = user_agent_alias
  end

  def get
    @agent.get(@url)
  end
end

