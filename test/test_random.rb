require 'test/unit'
require 'html/template/pro'

class TestRandom < Test::Unit::TestCase
  def test_random
    text = %{<TMPL_IF foo>YES<TMPL_ELSE>NO</TMPL_IF>}

    template = HTML::Template::Pro.new(:scalarref => text)
    template.param(:foo) do
      rand(2).to_i == 0 ? false : true
    end
    before, changed = template.output, false
    200.times do
      r = template.output
      changed = true if before != r
      assert (r == 'YES' || r == 'NO')
      before = r
    end
    assert changed, 'changed'
  end
end
