require "test/unit"

$:.unshift File.dirname(__FILE__) + "/../ext/html_template_internal"
require "html_template_internal.so"

class TestHtmlTemplateInternalExtn < Test::Unit::TestCase
  def test_truth
    assert(true)
  end
end