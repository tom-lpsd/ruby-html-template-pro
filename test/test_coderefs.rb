#!/usr/bin/env ruby

require 'test/unit'
require 'html/template/pro'

class TestCodeRefs < Test::Unit::TestCase
  def test_coderefs
    template = HTML::Template::Pro.new(:path     => 'templates',
                                       :filename => 'escape.tmpl')
    template.param(:STUFF) do
      '<>"\''
    end
    output = template.output
    assert_no_match(/[<>"']/, output)
  end
end
