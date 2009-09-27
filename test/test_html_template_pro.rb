#!/usr/bin/env ruby
# -*- coding: iso-8859-1 -*-

require 'test/unit'
require 'html/template/pro'

class TestHtmlTemplatePro < Test::Unit::TestCase

  @@varset1 = {
    :VAR1 => 'VAR1', :VAR2  => 'VAR2',
    :VAR3 => 'VAR3', :VAR10 => 'VAR10',
  }

  @@varset2 = {
    :STUFF1 => '\<>"; %FA' + "hidden:\r\012end",
    :STUFF2 => 'Some"' + "' Txt'",
  }

  @@refset1 = {
    :HASHREF0 => [],
    :HASHREF2 => [{}, {}],
    :HASHREF1 => [
                  { :LOOPVAR1=>'LOOP1-VAR1',:LOOPVAR2=>'LOOP1-VAR2',
                    :LOOPVAR3=>'LOOP1-VAR3',:LOOPVAR10=>'LOOP1-VAR10'},
                  { :LOOPVAR1=>'LOOP2-VAR1',:LOOPVAR2=>'LOOP2-VAR2',
                    :LOOPVAR3=>'LOOP2-VAR3',:LOOPVAR10=>'LOOP2-VAR10'},
                  { :LOOPVAR1=>'LOOP3-VAR1',:LOOPVAR2=>'LOOP3-VAR2',
                    :LOOPVAR3=>'LOOP3-VAR3',:LOOPVAR10=>'LOOP3-VAR10'},
                  { :LOOPVAR1=>'LOOP4-VAR1',:LOOPVAR2=>'LOOP4-VAR2',
                    :LOOPVAR3=>'LOOP4-VAR3',:LOOPVAR10=>'LOOP4-VAR10'},
                 ],
  }

  @@outer = [{:TEST=>'1'},{:TEST=>'2'},{:TEST=>'3'}]
  @@inner = [{:TST=>'A'},{:TST=>'B'}]

  @@refset2 = { :INNER => @@inner, :OUTER => @@outer }

  def test_tmplpro
    test_tmpl_std('test_esc1', @@varset1, @@varset2)
    test_tmpl_std('test_esc2', @@varset1, @@varset2)
    test_tmpl_std('test_esc3', @@varset1, @@varset2)
    test_tmpl_std('test_esc4', @@varset1, @@varset2)
    test_tmpl_std('test_var1', @@varset1)
    test_tmpl_std('test_var2', @@varset1)
    test_tmpl_std('test_var3', @@varset1, @@varset2)
    test_tmpl_std('test_if1',  @@varset1)
    test_tmpl_std('test_if2',  @@varset1)
    test_tmpl_std('test_if3',  @@refset1)
    test_tmpl_std('test_if4',  @@varset1)
    test_tmpl_std('test_if5',  @@varset1)
    test_tmpl_std('test_if7',  @@varset1)
    test_tmpl_std('test_include1', @@varset1)
    test_tmpl('test_include2', { :max_include => 10 }, @@varset1)
    test_tmpl_std('test_include3', @@varset1)
    test_tmpl('test_include4', { :path => ['include/1', 'include/2'] });
    test_tmpl('test_include5', {
                :path => ['include/1', 'include/2'],
                :search_path_on_include => 1,
              });
    test_tmpl_std('test_loop1', @@varset1, @@refset1)
    test_tmpl_std('test_loop2', @@varset1, @@refset1)
    test_tmpl_std('test_loop3', @@varset1, @@refset1)
    test_tmpl_std('test_loop4', @@varset1, @@refset1)
    test_tmpl_std('test_loop5', @@varset1, @@refset1)
    test_tmpl('test_loop6', {
                         :loop_context_vars => true,
                         :global_vars => true,
              }, @@refset2)

    test_tmpl_std('include/2', { :LIST => [{ :TEST => 1 }, { :TEST => 2 }] });
    test_tmpl('test_broken1', { :debug => -1 }, @@varset1, @@refset1);

    assert true
  end

  def test_tmplpro_expr

    HTML::Template::Pro.register_function(:registered_func) { |x| x  }
    HTML::Template::Pro.register_function(:hello_string) { 'hello!' }
    HTML::Template::Pro.register_function(:arglist) { |*args|  '[' + args.join('][') + ']' }
    HTML::Template::Pro.register_function(:f1) { |*args| "F1: #{args.join(' ')}" }
    HTML::Template::Pro.register_function(:f2) { |*args| "F2: #{args.join(' ')}" }
    HTML::Template::Pro.register_function(:fUNDEF) { nil }

    exprset1 = {
      :ONE=>1,  :TWO=>2, :THREE=>3, :ZERO=>0, :MINUSTEN=>-10,
      :FILE=>'test_if1.tmpl', :TWENTY=>20, :FOURTY=>50,
    }

    brunoext = { :'FOO.BAR' => '<test passed>' }

    refset1 = {
      :HASHREF0 => [],
      :HASHREF2 => [{},{}],
      :HASHREF1 => [
                    {:LOOPVAR1=>'LOOP1-VAR1',:LOOPVAR2=>'LOOP1-VAR2',:LOOPVAR3=>'LOOP1-VAR3',:LOOPVAR10=>'LOOP1-VAR10'},
                    {:LOOPVAR1=>'LOOP2-VAR1',:LOOPVAR2=>'LOOP2-VAR2',:LOOPVAR3=>'LOOP2-VAR3',:LOOPVAR10=>'LOOP2-VAR10'},
                    {:LOOPVAR1=>'LOOP3-VAR1',:LOOPVAR2=>'LOOP3-VAR2',:LOOPVAR3=>'LOOP3-VAR3',:LOOPVAR10=>'LOOP3-VAR10'},
                    {:LOOPVAR1=>'LOOP4-VAR1',:LOOPVAR2=>'LOOP4-VAR2',:LOOPVAR3=>'LOOP4-VAR3',:LOOPVAR10=>'LOOP4-VAR10'},
                   ]
    }

    test_tmpl_std('test_expr1', exprset1);
    test_tmpl_std('test_expr2', exprset1);
    test_tmpl_std('test_expr3', exprset1);
    test_tmpl_std('test_expr4', brunoext);
    test_tmpl_std('test_expr5', exprset1);
    test_tmpl_std('test_expr6', exprset1);
    test_tmpl_std('test_expr7', refset1);
    test_tmpl_std('test_expr8', exprset1);

    test_tmpl_expr('test_userfunc1', exprset1)
    test_tmpl_expr('test_userfunc2', exprset1)
    test_tmpl_expr('test_userfunc3', exprset1)
    test_tmpl_expr('test_userfunc4', exprset1)
    test_tmpl_expr('test_userfunc5', exprset1)
    test_tmpl_expr('test_userfunc6', exprset1)

  end

  private

  def test_tmpl(file, option, *args)
    params = args.inject({}) do |result, item|
      result.update(item)
    end
    Dir.chdir 'templates-Pro'
    tmpl = HTML::Template::Pro.new({ :filename => file + '.tmpl',
                                     :debug => $DEBUG || 0 }.merge(option) )
    tmpl.param(params)
    dryrun(tmpl, file);
    Dir.chdir '..'
  end

  @@case_ext = {
    :loop_context_vars => true,
    :case_sensitive => false,
  }

  @@case_int = {
    :loop_context_vars => true,
    :case_sensitive => true,
    :tmpl_var_case => HTML::Template::Pro::ASK_NAME_UPPERCASE,
  }

  def test_tmpl_std(file, *args)
    test_tmpl(file, @@case_ext, *args)
    test_tmpl(file, @@case_int, *args)
  end

  def test_tmpl_expr(file, *args)
    Dir.chdir 'templates-Pro'
    tmpl = HTML::Template::Pro.new(:filename => file + '.tmpl',
                                   :loop_context_vars => true,
                                   :case_sensitive => true,
                                   :tmpl_var_case => HTML::Template::Pro::ASK_NAME_UPPERCASE | HTML::Template::Pro::ASK_NAME_AS_IS,
                                   :debug => $DEBUG || 0,
                                   :functions => { :hello => lambda { |x| "hi, #{x}!" } }
                                   )
    tmpl.param(*args)
    tmpl.register_function(:per_object_call) { |x| "#{x}-arg" }
    tmpl.register_function(:perobjectcall2) { |x| "#{x}-arg"}
    dryrun(tmpl, file)
    Dir.chdir '..'
  end

  def dryrun(tmpl, file)
    File.open("#{file}.raw", 'w') do |f|
      tmpl.output(:print_to => f)
    end
    assert_equal File.read("#{file}.out"), File.read("#{file}.raw")
    File.unlink("#{file}.raw")
    output = tmpl.output;
    assert_equal File.read("#{file}.out"), output
  end
end
