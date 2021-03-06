require 'rubygems'
gem "test-unit"
require "test/unit"

ENV["RAILS_ENV"] = "test"
require File.expand_path(File.dirname(__FILE__) + "/../config/environment")
require 'test_help'

require 'webrat'

Webrat.configure do |config|
  config.mode = :rails
end

class ActiveSupport::TestCase
  # Transactional fixtures accelerate your tests by wrapping each test method
  # in a transaction that's rolled back on completion.  This ensures that the
  # test database remains unchanged so your fixtures don't have to be reloaded
  # between every test method.  Fewer database queries means faster tests.
  #
  # Read Mike Clark's excellent walkthrough at
  #   http://clarkware.com/cgi/blosxom/2005/10/24#Rails10FastTesting
  #
  # Every Active Record database supports transactions except MyISAM tables
  # in MySQL.  Turn off transactional fixtures in this case; however, if you
  # don't care one way or the other, switching from MyISAM to InnoDB tables
  # is recommended.
  #
  # The only drawback to using transactional fixtures is when you actually 
  # need to test transactions.  Since your test is bracketed by a transaction,
  # any transactions started in your code will be automatically rolled back.
  self.use_transactional_fixtures = true

  # Instantiated fixtures are slow, but give you @david where otherwise you
  # would need people(:david).  If you don't want to migrate your existing
  # test cases which use the @david style and don't mind the speed hit (each
  # instantiated fixtures translates to a database query per test method),
  # then set this back to true.
  self.use_instantiated_fixtures  = false

  # Setup all fixtures in test/fixtures/*.(yml|csv) for all tests in alphabetical order.
  #
  # Note: You'll currently still have to declare fixtures explicitly in integration tests
  # -- they do not yet inherit this setting
  fixtures :all

  set_fixture_class(:milters => "Config::Milter",
                    :connections => "Config::Connection",
                    :applicable_conditions => "Config::ApplicableCondition",
                    :restrictions => "Config::Restriction")

  # Add more helper methods to be used by all tests here...
  def assert_valid_model(model)
    model.save
    assert_equal({}, model.errors.instance_variable_get("@errors"))
  end

  def assert_invalid_model(expected_errors, model)
    model.save
    actual_errors = {}
    model.errors.each do |name, message|
      actual_errors[name] ||= []
      actual_errors[name] << message
    end
    assert_equal(expected_errors, actual_errors)
  end

  def t(key, options=nil)
    I18n.translate(key, options || {})
  end

  def t_ar(key, options=nil)
    options ||= {}
    t(key, options.merge(:scope => [:activerecord]))
  end
end

module ActionController
  class TestRequest
    def accept_language
      @env["HTTP_ACCEPT_LANGUAGE"] || "ja,en;q=0.5"
    end

    def accept_language=(language)
      @env["HTTP_ACCEPT_LANGUAGE"] = language
    end
  end

  class IntegrationTest
    def t(*args)
      I18n.translate(*args)
    end

    def open_session(&block)
      session = super(&block)
      def session.process(method, path, parameters=nil, headers=nil)
        headers = (headers || {}).dup
        headers["HTTP_ACCEPT_LANGUAGE"] ||= "ja,en;q=0.5"
        super(method, path, parameters, headers)
      end
      session
    end

    private
    def login
      user = users(:aaron)
      visit(root_path)
      fill_in(t("label.login"), :with => user.login)
      fill_in(t("label.password"), :with => "monkey")
      click_button(t("action.login"))
    end
  end
end
