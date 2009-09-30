require 'test/unit'
require 'html/template/pro'

class TestPathLikeVariableScope < Test::Unit::TestCase
  def test_path_like_variable_scope

    template_text = <<'END'
<TMPL_LOOP NAME=class>
  <TMPL_LOOP NAME=person>
    <TMPL_VAR NAME="../teacher_name">  <!-- access to class.teacher_name -->
    <TMPL_VAR NAME="name">
    <TMPL_VAR NAME="age">
    <TMPL_VAR NAME="/top_level_value"> <!-- access to top level value -->
    <TMPL_VAR EXPR="${/top_level_value} * 5"> <!-- need ${} to use path_like_variable in EXPR -->
  </TMPL_LOOP>
</TMPL_LOOP>
END

    template = HTML::Template::Pro.new(:path_like_variable_scope => true,
                                       :scalarref => template_text)
    template.param(:top_level_value => "3",
                   :class => [
                              {
                                :teacher_name => "Adam",
                                :person => [
                                            {
                                              :name => "Jon",
                                              :age  => "20",
                                            },
                                            {
                                              :name => "Bob",
                                              :age  => "21",
                                            },
                                           ],
                              },
                              {
                              }
                             ])
    assert_equal(<<'END', template.output)

  
    Adam  <!-- access to class.teacher_name -->
    Jon
    20
    3 <!-- access to top level value -->
    15 <!-- need ${} to use path_like_variable in EXPR -->
  
    Adam  <!-- access to class.teacher_name -->
    Bob
    21
    3 <!-- access to top level value -->
    15 <!-- need ${} to use path_like_variable in EXPR -->
  

  

END
  end
end
