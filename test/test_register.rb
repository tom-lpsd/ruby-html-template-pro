require 'test/unit'
require 'html/template/pro'

class TestRegister < Test::Unit::TestCase
  def test_register
    HTML::Template::Expr.register_function(:directory_exists) { |dir| FileTest.directory? dir }
    HTML::Template::Expr.register_function(:commify => lambda do |x|
                                             x = x.to_s
                                             1 while x.gsub!(/^([-+]?\d+)(\d{3})/, '\1,\2')
                                             return x
                                           end)

    template = HTML::Template::Expr.new(:path     => ['test/templates'],
                                        :filename => 'register.tmpl')
    output = template.output
    assert_match /^OK/, output, 'directory_exists()'
    assert_match /2,000/, output, 'comify'

    begin
      HTML::Template::Expr.register_function(:foo => 'bar');
    rescue => e
      assert_match /must be kind_of Proc/, e.message, 'type check'
    end
  end
end
