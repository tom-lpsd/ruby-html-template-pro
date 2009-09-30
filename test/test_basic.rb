require 'test/unit'
require 'html/template/pro'

class TestBasic < Test::Unit::TestCase
  def test_basic
    template = HTML::Template::Expr.new(:path     => ['test/templates'],
                                       :filename => 'foo.tmpl')
    template.param(:foo => 100)
    output = template.output()
    assert_match(/greater than/i, output, "greater than")

    template.param(:foo => 10)
    output = template.output()
    assert_match(/less than/i, output, "less than")

    template = HTML::Template::Expr.new(:path => ['test/templates'],
                                        :filename => 'negative.tmpl')
    template.param(:foo => 100)
    output = template.output()
    assert_match(/Yes/, output, "negative numbers work")

  end
end
