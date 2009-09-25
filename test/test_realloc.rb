require 'test/unit'
require 'html/template/pro'

class TestRealloc < Test::Unit::TestCase
  def test_realloc
    mult  = 10
    tests = 30
    t = HTML::Template::Pro.new( :filename => 'templates-Pro/test_malloc.tmpl',
                                 :debug=>0 )
    25.step(25+tests*mult-1, mult) do |x|
      txt = 'xxxxxxxxxx' * x
      t.param(:text => txt)
      assert_equal((txt +"\n"), t.output)
    end
  end
end
