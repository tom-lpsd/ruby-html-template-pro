require 'test/unit'
require 'html/template/pro'

class TestComplex < Test::Unit::TestCase
  def test_complex

    template = HTML::Template::Expr.new(:path     => ['test/templates'],
                                        :filename => 'complex.tmpl')
    template.param(:foo => 11,
                   :bar => 0,
                   :fname => 'president',
                   :lname => 'clinton',
                   :unused => 0,
                   )

    output = template.output()

    assert_match /Foo is greater than 10/i, output, "greater than"
    assert_no_match /Bar and Foo/i, output, "and"
    assert_match /Bar or Foo/i, output, "or"
    assert_match /Bar - Foo = -11/i, output, "subtraction"
    assert_match /Math Works, Alright/i, output, "math"
    assert_match /My name is President Clinton/, output, "string op 1"
    assert_match /Resident Alien is phat/, output, "string op 2"
    assert_match /Resident has 8 letters, which is less than 10 and greater than 5/, output, "string length"

    template = HTML::Template::Expr.new(:path     => ['test/templates'],
                                        :filename => 'loop.tmpl',
                                        :global_vars => 1)
    template.param(:simple => [
                               { :foo => 10 },
                               { :foo => 100 },
                               { :foo => 1000 },
                              ])
    template.param(:color => 'blue')
    template.param(:complex => [ 
                                { 
                                  :fname => 'Yasunari',
                                  :lname => 'Kawabata',
                                  :inner => [
                                             { :stat_name => 'style', 
                                               :stat_value => 100 ,
                                             },
                                             { :stat_name => 'shock',
                                               :stat_value => 1,
                                             },
                                             { :stat_name => 'poetry',
                                               :stat_value => 100
                                             },
                                             { :stat_name => 'machismo',
                                               :stat_value => 50
                                             },
                                            ],
                                },
                              { 
                                  :fname => 'Yukio',
                                  :lname => 'Mishima',
                                  :inner => [
                                             { :stat_name => 'style', 
                                               :stat_value => 50,
                                             },
                                             { :stat_name => 'shock',
                                               :stat_value => 100,
                                             },
                                             { :stat_name => 'poetry',
                                               :stat_value => 1
                                             },
                                             { :stat_name => 'machismo',
                                               :stat_value => 100
                                             },
                                            ],
                                },
                               ])

    output = template.output
    assert_match /Foo is less than 10.\s+Foo is greater than 10.\s+Foo is greater than 10./, output, "math in loops"

    # test user-defined functions
    repeat = ->(x, y) { x * y }

    template = HTML::Template::Expr.new(:path => ['test/templates'],
                                        :filename => 'func.tmpl',
                                        :functions => {
                                          :repeat => repeat,
                                        })
    template.param(:repeat_me => 'foo ');
    output = template.output();

    assert_match /foo foo foo foo/, output, "user defined function"
    assert_match /FOO FOO FOO FOO/, output, "user defined function with uc()"

    # test numeric functions
    template = HTML::Template::Expr.new(:path => ['test/templates'],
                                        :filename => 'numerics.tmpl')
    template.param(:float => 5.1,
                   :four => 4)
    output = template.output

    assert_match /INT: 5/, output, "int()"
    assert_match /SQRT: 2/, output, "sqrt()"
    assert_match /SQRT2: 4/, output, "sqrt() 2"
    assert_match /SUM: 14/, output, "int(4 + 10.1)"
    assert_match /SPRINTF: 14.1000/, output, "sprintf('%0.4f', (10.1 + 4))"

  end
end
