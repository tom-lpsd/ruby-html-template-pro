require 'test/unit'
require 'html/template/pro'

class TestHtmlTemplate < Test::Unit::TestCase
  def test_tmplpro
    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'simple.tmpl',
                                       :debug => 0)
    template.param(:ADJECTIVE => 'very')
    output = template.output
    assert(output !~ /ADJECTIVE/ && template.param(:ADJECTIVE) == 'very')

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'medium.tmpl',
                                       )

    template.param(:ALERT => 'I am alert.')
    template.param(:COMPANY_NAME => "MY NAME IS")
    template.param(:COMPANY_ID => "10001")
    template.param(:OFFICE_ID => "10103214")
    template.param(:NAME => 'SAM I AM')
    template.param(:ADDRESS => '101011 North Something Something')
    template.param(:CITY => 'NEW York')
    template.param(:STATE => 'NEw York')
    template.param(:ZIP =>'10014')
    template.param(:PHONE =>'212-929-4315')
    template.param(:PHONE2 =>'')
    template.param(:SUBCATEGORIES =>'kfldjaldsf')
    template.param(:DESCRIPTION =>"dsa;kljkldasfjkldsajflkjdsfklfjdsgkfld\nalskdjklajsdlkajfdlkjsfd\n\talksjdklajsfdkljdsf\ndsa;klfjdskfj")
    template.param(:WEBSITE =>'http://www.assforyou.com/')
    template.param(:INTRANET_URL =>'http://www.something.com')
    template.param(:REMOVE_BUTTON => "<INPUT TYPE=SUBMIT NAME=command VALUE=\"Remove Office\">")
    template.param(:COMPANY_ADMIN_AREA => "<A HREF=administrator.cgi?office_id={office_id}&command=manage>Manage Office Administrators</A>")
    template.param(:CASESTUDIES_LIST => "adsfkljdskldszfgfdfdsgdsfgfdshghdmfldkgjfhdskjfhdskjhfkhdsakgagsfjhbvdsaj hsgbf jhfg sajfjdsag ffasfj hfkjhsdkjhdsakjfhkj kjhdsfkjhdskfjhdskjfkjsda kjjsafdkjhds kjds fkj skjh fdskjhfkj kj kjhf kjh sfkjhadsfkj hadskjfhkjhs ajhdsfkj akj fkj kj kj  kkjdsfhk skjhadskfj haskjh fkjsahfkjhsfk ksjfhdkjh sfkjhdskjfhakj shiou weryheuwnjcinuc 3289u4234k 5 i 43iundsinfinafiunai saiufhiudsaf afiuhahfwefna uwhf u auiu uh weiuhfiuh iau huwehiucnaiuncianweciuninc iuaciun iucniunciunweiucniuwnciwe")
    template.param(:NUMBER_OF_CONTACTS => "aksfjdkldsajfkljds")
    template.param(:COUNTRY_SELECTOR => "klajslkjdsafkljds")
    template.param(:LOGO_LINK => "dsfpkjdsfkgljdsfkglj")
    template.param(:PHOTO_LINK => "lsadfjlkfjdsgkljhfgklhasgh")

    output = template.output;
    assert_no_match /<TMPL_VAR/, output

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'simple-loop.tmpl',
                                       )

    template.param(:ADJECTIVE_LOOP => [ { :ADJECTIVE => 'really' },
                                        { :ADJECTIVE => 'very' } ] )

    output = template.output
    assert_no_match /ADJECTIVE_LOOP/, output
    assert_match /really.*very/m, output

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'simple-loop-nonames.tmpl',
                                       )
    
    template.param(:ADJECTIVE_LOOP => [ { :ADJECTIVE => 'really' },
                                        { :ADJECTIVE => 'very' } ] )

    output = template.output
    assert_no_match /ADJECTIVE_LOOP/, output
    assert_match /really.*very/m, output

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'long_loops.tmpl',
                                       )
    output = template.output
    assert true

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'include.tmpl',
                                       )
    output = template.output
    assert_match /5/, output
    assert_match /6/, output

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'include.tmpl',
                                       :cache => true,
                                       )
    output = template.output
    assert_match /5/, output
    assert_match /6/, output

    template_one = HTML::Template::Pro.new(:path => 'templates',
                                           :filename => 'simple.tmpl',
                                           )
    template_one.param(:ADJECTIVE => 'very')

    template_two = HTML::Template::Pro.new(:path => 'templates',
                                           :filename => 'simple.tmpl',
                                           :associate => template_one,
                                           )
    output = template_two.output
    assert_no_match /ADJECTIVE/, output
    assert_match /very/, output

    template_l = HTML::Template::Pro.new(:path => 'templates',
                                         :filename => 'other-loop.tmpl',
                                         )
    output = template_l.output
    assert_no_match /INSIDE/, output

    template_i = HTML::Template::Pro.new(:path => 'templates',
                                         :filename => 'if.tmpl',
                                         )
    output = template_i.output
    assert_no_match /INSIDE/, output

    template_i2 = HTML::Template::Pro.new(:path => 'templates',
                                         :filename => 'if.tmpl',
                                         )
    template_i2.param(:BOOL => 1)
    output = template_i2.output
    assert_match /INSIDE/, output

    template_ie = HTML::Template::Pro.new(:path => 'templates',
                                          :filename => 'ifelse.tmpl',
                                          )
    output = template_ie.output
    assert_match /INSIDE ELSE/, output

    template_ie2 = HTML::Template::Pro.new(:path => 'templates',
                                           :filename => 'ifelse.tmpl',
                                           )
    template_ie2.param(:BOOL => 1)
    output = template_ie2.output
    assert_match /INSIDE IF/, output
    assert_no_match /INSIDE ELSE/, output

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'double_loop.tmpl',
                                       )
    template.param(:myloop => [
                               { :var => 'first'}, 
                               { :var => 'second' }, 
                               { :var => 'third' }
                              ]
                   )

    output = template.output
    assert_match /David/, output

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'escape.tmpl',
                                       )
    template.param(:STUFF => '<>"\'')
    output = template.output
    assert_no_match /[<>\"\']/, output

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'simple.tmpl',
                                       )
    template.param({ :ADJECTIVE => 'very' })
    output = template.output
    assert_no_match /ADJECTIVE/, output
    assert_match /very/, output

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'unless.tmpl',
                                       )
    template.param(:BOOL => true)
    output = template.output
    assert_no_match /INSIDE UNLESS/, output
    assert_match /INSIDE ELSE/, output

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'unless.tmpl',
                                       )
    template.param(:BOOL => false)
    output = template.output
    assert_match /INSIDE UNLESS/, output
    assert_no_match /INSIDE ELSE/, output

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'context.tmpl',
                                       :loop_context_vars => true,
                                       )
    template.param(:FRUIT => [
                              {:KIND => 'Apples'},
                              {:KIND => 'Oranges'},
                              {:KIND => 'Brains'},
                              {:KIND => 'Toes'},
                              {:KIND => 'Kiwi'}
                             ])
    template.param(:PINGPONG => [ {}, {}, {}, {}, {}, {} ])

    output = template.output
    assert_match /Apples, Oranges, Brains, Toes, and Kiwi./, output
    assert_match /pingpongpingpongpingpong/, output

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'loop-if.tmpl',
                                       )
    output = template.output
    assert_match /Loop not filled in/, output

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'loop-if.tmpl',
                                       )
    template.param(:LOOP_ONE => [{:VAR => "foo"}])
    output = template.output
    assert_no_match /Loop not filled in/, output

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'simple.tmpl',
                                       :debug => 0,
                                       )
    template.param(:ADJECTIVE) { 'v' + '1e' + '2r' + '3y' }
    output = template.output
    assert_match /v1e2r3y/, output


    template = HTML::Template::Pro.new(:path => ['templates'],
                                       :filename => 'simple.tmpl',
                                       :cache => 1,
                                       :debug => 0,
                                       )
    template.param(:ADJECTIVE) { 'v' + '1e' + '2r' + '3y' }
    output = template.output
    template = HTML::Template::Pro.new(:path => ['templates/'],
                                      :filename => 'simple.tmpl',
                                      :cache => 1,
                                      :debug => 0,
                                      )
    assert_match /v1e2r3y/, output

    template = HTML::Template::Pro.new(:path => ['templates/'],
                                      :filename => 'urlescape.tmpl',
                                      )
    template.param(:STUFF => '<>"; %FA')
    output = template.output
    assert_no_match /[<>\"]/, output

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'globals.tmpl',
                                       :global_vars => true,
                                       )
    template.param(:outer_loop => [{:loop => [{:LOCAL => 'foo'}]}])
    template.param(:global => 'bar')
    template.param(:hidden_global => 'foo')
    result = template.output
    assert_match /foobar/, result

    template = HTML::Template::Pro.new(:path => 'templates',
                                       :filename => 'loop-context.tmpl',
                                       :loop_context_vars => true,
                                       )
    template.param(:TEST_LOOP => [ { :NUM => 1 } ])
    result = template.output
    assert_match /1:FIRST::LAST:ODD/, result

    template = HTML::Template::Pro.new(:path => ['templates/searchpath',
                                                 'templates'],
                                       :filename => 'include.tmpl',
                                       :search_path_on_include => true,
                                       )
    output = template.output
    assert_match /9/, output
    assert_match /6/, output

    template = HTML::Template::Pro.new(:path => ['templates'],
                                       :filename => 'simple.tmpl',
                                       :file_cache_dir => './blib/temp_cache_dir',
                                       :file_cache => true,
                                       )
    template.param(:ADJECTIVE) { "3y" }
    output = template.output
    template = HTML::Template::Pro.new(:path => ['templates/'],
                                       :filename => 'simple.tmpl',
                                       :file_cache_dir => './blib/temp_cache_dir',
                                       :file_cache => true,
                                       )
    assert_match /3y/, output

    x = nil
    template = HTML::Template::Pro.new(:filename => 'templates/simple-loop.tmpl',
                                       :filter => {
                                         :sub => lambda { |args|
                                           x = 1
                                           args.each do |e|
                                             e = "#{x} : #{e}"
                                             x+=1
                                           end
                                         },
                                         :format => 'array',
                                       },
                                       :file_cache_dir => './blib/temp_cache_dir',
                                       :file_cache => true,
                                       :global_vars => true,
                                       )
    template.param(:ADJECTIVE_LOOP => [ { :ADJECTIVE => 'really' },
                                        { :ADJECTIVE => 'very' } ] )
    output = template.output
    assert_match /very/, output

    template = HTML::Template::Pro.new(:filename => './templates/include_path/a.tmpl')
    output = template.output
    assert_match /Bar/, output

    File.open('test.out', 'w') do |file|
      template = HTML::Template::Pro.new(:filename => './templates/include_path/a.tmpl')
      template.output(:print_to => file);
    end
    File.open('test.out', 'r') do |file|
      output = file.read
    end
    assert_match /Bar/, output
    File.delete('test.out')

    template_source = <<END_OF_TMPL
  I am a <TMPL_VAR NAME="adverb"> <TMPL_VAR NAME="ADVERB"> simple template.
END_OF_TMPL
    
    template = HTML::Template::Pro.new(
                                       :scalarref => template_source,
                                       :case_sensitive => true,
                                       :debug => 0,
                                       )
    template.param(:adverb => 'very')
    template.param(:ADVERB => 'painfully')
    output = template.output
    assert_no_match /ADVERB/i, output
    assert_equal 'painfully', template.param(:ADVERB)
    assert_equal 'very', template.param(:adverb)
    assert_match /very painfully/, output

    template_source = <<END_OF_TMPL;
  I am a <TMPL_VAR NAME="adverb"> <TMPL_VAR NAME="ADVERB"> simple template.
END_OF_TMPL
    template = HTML::Template::Pro.new(
                                       :scalarref => template_source,
                                       :case_sensitive => false,
                                       :debug => 0,
                                       )

    template.param(:adverb => 'very')
    template.param(:ADVERB => 'painfully')
    output = template.output
    assert_no_match /ADVERB/i, output
    assert_equal 'painfully', template.param(:ADVERB)
    assert_equal 'painfully', template.param(:adverb)
    assert_match /painfully painfully/, output

    template = HTML::Template::Pro.new(:filename => './templates/include_path/a.tmpl',
                                       :filter => lambda { |src|
                                         src.gsub(/Bar/, 'Zanzabar')
                                       },
                                       )
    output = template.output
    assert_match /Zanzabar/, output

    template = HTML::Template::Pro.new(:filename => './templates/include_path/a.tmpl',
                                       :filter => [
                                                   {
                                                     :sub => lambda { |src|
                                                       src.gsub(/Bar/, 'Zanzabar')
                                                     },
                                                     :format => 'scalar'
                                                   },
                                                   {
                                                     :sub => lambda { |src|
                                                       src.gsub(/bar/, 'bar!!!')
                                                     },
                                                     :format => 'scalar'
                                                   }
                                                  ]
                                       )
    output = template.output
    assert_match /Zanzabar!!!/, output

    template = HTML::Template::Pro.new(:filename => './templates/include_path/a.tmpl',
                                       :filter => {
                                         :sub => lambda { |args|
                                           x = 1
                                           args.map do |e|
                                             e = "#{x} : #{e}"
                                             x+=1
                                             e
                                           end
                                         },
                                         :format => 'array',
                                       },
                                       )
    output = template.output
    assert_match /1 : Foo/, output

    template = HTML::Template::Pro.new(
                                       :scalarref => "\n<TMPL_INCLUDE templates/simple.tmpl>",
                                       )
    template.param(:ADJECTIVE => "3y")
    output = template.output
    assert_match /3y/, output

    template = HTML::Template::Pro.new(:path => ['templates'],
                                       :filename => 'newline_test1.tmpl',
                                       :filter => lambda { |src| src },
                                       )
    output = template.output
    assert_match /STARTincludeEND/, output

    template = HTML::Template::Pro.new(:path => ['templates'],
                                       :filename => 'multiline_tags.tmpl',
                                       :global_vars => true,
                                       )
    template.param(:FOO => 'foo!', :bar => [{}, {}])
    output = template.output
    assert_match /foo!\n/, output
    assert_match /foo!foo!\nfoo!foo!/, output

    # test new() from filehandle
    File.open("templates/simple.tmpl") do |file|
      template = HTML::Template::Pro.new(:filehandle => file)
      template.param(:ADJECTIVE => 'very')
      output = template.output
      assert_no_match /ADJECTIVE/, output
      assert_equal 'very', template.param(:ADJECTIVE)
    end

    # test new_() from filehandle
    File.open("templates/simple.tmpl") do |file|
      template = HTML::Template::Pro.new_filehandle(file)
      template.param(:ADJECTIVE => 'very')
      output = template.output
      assert_no_match /ADJECTIVE/, output
      assert_equal 'very', template.param(:ADJECTIVE)
    end

    # test case sensitive loop variables
    template = HTML::Template::Pro.new(:path => ['templates'],
                                       :filename => 'case_loop.tmpl',
                                       :case_sensitive => true,
                                       );
    template.param(:loop => [ { :foo => 'bar', :FOO => 'BAR' } ])
    output = template.output
    assert_match /bar BAR/, output

    # test ifs with code refd
    template = HTML::Template::Pro.new(:path => ['templates'],
                                       :filename => 'if.tmpl')
    template.param(:bool) { false }
    output = template.output
    assert_no_match /INSIDE/, output
    assert_match /unless/, output

    # test global_vars for loops within loops
    template = HTML::Template::Pro.new(:path => ['templates'],
                                       :filename => 'global-loops.tmpl',
                                       :global_vars => true)
    template.param(:global => "global val")
    template.param(:outer_loop => [
                                   { 
                                     :foo => 'foo val 1',
                                     :inner_loop => [
                                                     { :bar => 'bar val 1' },
                                                     { :bar => 'bar val 2' },
                                                    ],
                                   },
                                   {
                                     :foo => 'foo val 2',
                                     :inner_loop => [
                                                     { :bar => 'bar val 3' },
                                                     { :bar => 'bar val 4' },
                                                    ],
                                   }
                                  ])
    output = template.output
    assert_match /inner loop foo:    foo val 1/, output
    assert_match /inner loop foo:    foo val 2/, output

    # test nested include path handling
    template = HTML::Template::Pro.new(:path => ['templates'],
                                       :filename => 'include_path/one.tmpl')
    output = template.output
    assert_match /ONE/, output
    assert_match /TWO/, output
    assert_match /THREE/, output

    # test using HTML_TEMPLATE_ROOT with path
    ENV['HTML_TEMPLATE_ROOT'] = 'templates'
    template = HTML::Template::Pro.new(
                                       :path => ['searchpath'],
                                       :filename => 'three.tmpl',
                                       )
    output = template.output
    assert_match /THREE/, output
    ENV.delete('HTML_TEMPLATE_ROOT')

    # test __counter__
    template = HTML::Template::Pro.new(:path              => ['templates'],
                                       :filename          => 'counter.tmpl',
                                       :loop_context_vars => true)
    template.param(:foo => [ {:a => 'a'}, {:a => 'b'}, {:a => 'c'} ])
    template.param(:outer => [ {:inner => [ {:a => 'a'}, {:a => 'b'}, {:a => 'c'} ] },
                               {:inner => [ {:a => 'x'}, {:a => 'y'}, {:a => 'z'} ] },
                             ])
    output = template.output
    assert_match /^1a2b3c$/m, output
    assert_match /^11a2b3c21x2y3z$/m, output

    # test default
    template = HTML::Template::Pro.new(:path              => ['templates'],
                                       :filename          => 'default.tmpl')
    template.param(:cl => 'clothes')
    template.param(:start => 'start')
    output = template.output
    assert_match /cause it\'s getting hard to think, and my clothes are starting to shrink/, output

    # test a case where a different path should stimulate a cache miss
    # even though the main template is the same
    template = HTML::Template::Pro.new(:path => ['templates', 
                                                 'templates/include_path'],
                                       :filename => 'outer.tmpl',
                                       :search_path_on_include => true,
                                       :cache => true,
                                       )
    output = template.output
    assert_match /I AM OUTER/, output
    assert_match /I AM INNER 1/, output

    template = HTML::Template::Pro.new(:path => ['templates', 
                                                 'templates/include_path2'],
                                       :filename => 'outer.tmpl',
                                       :search_path_on_include => true,
                                       :cache => true,
                                       )
    output = template.output
    assert_match /I AM OUTER/, output
    assert_match /I AM INNER 2/, output

    # try the same thing with the file cache
    template = HTML::Template::Pro.new(:path => ['templates', 
                                                 'templates/include_path'],
                                       :filename => 'outer.tmpl',
                                       :search_path_on_include => true,
                                       :file_cache_dir => './blib/temp_cache_dir',
                                       :file_cache => true,
                                       )
    output = template.output
    assert_match /I AM OUTER/, output
    assert_match /I AM INNER 1/, output

    template = HTML::Template::Pro.new(:path => ['templates', 
                                                 'templates/include_path2'],
                                       :filename => 'outer.tmpl',
                                       :search_path_on_include => true,
                                       :file_cache_dir => './blib/temp_cache_dir',
                                       :file_cache => true,
                                       )
    output = template.output
    assert_match /I AM OUTER/, output
    assert_match /I AM INNER 2/, output

    # test javascript escaping
    template = HTML::Template::Pro.new(:path => ['templates'],
                                       :filename => 'js.tmpl')
    template.param(:msg => %{"He said 'Hello'.\n\r"})
    output = template.output
    assert_equal %q{\\"He said \\'Hello\\'.\\n\\r\\"}, output

    # test default escaping
    template = HTML::Template::Pro.new(:path => ['templates'],
                                       :filename => 'default_escape.tmpl',
                                       :default_escape => 'UrL')
    template.param(:STUFF => %q{Joined with space})
    output = template.output
    assert_match %r{^Joined%20with%20space}, output

    template = HTML::Template::Pro.new(:path => ['templates'],
                                       :filename => 'default_escape.tmpl',
                                       :default_escape => 'html')
    template.param(:STUFF => 'Joined&with"cruft')
    template.param(:LOOP => [ { :MORE_STUFF => '<&>' }, { :MORE_STUFF => '>&<' } ])
    template.param(:a => '<b>')
    output = template.output

    assert_match %r{^Joined&amp;with&quot;cruft}, output
    assert_match %r{&lt;&amp;&gt;&gt;&amp;&lt;}, output
    assert_match %r{because it\'s &lt;b&gt;}, output
  end
end

