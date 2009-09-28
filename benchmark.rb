require 'benchmark'
require 'erb'
require 'erubis'
require 'html/template/pro'

varset1 = { :VAR1 => 'VAR1', :VAR2 => 'VAR2', :VAR3 => 'VAR3', :VAR10 => 'VAR10' }
varset2 = { :STUFF1 => '<>"; %FA' }

refset1 = {
  :HASHREF0=>[],
  :HASHREF2=>[{},{}],
  :HASHREF1=>[
              {:LOOPVAR1=>'LOOP1-VAR1',:LOOPVAR2=>'LOOP1-VAR2',:LOOPVAR3=>'LOOP1-VAR3',:LOOPVAR10=>'LOOP1-VAR10'},
              {:LOOPVAR1=>'LOOP2-VAR1',:LOOPVAR2=>'LOOP2-VAR2',:LOOPVAR3=>'LOOP2-VAR3',:LOOPVAR10=>'LOOP2-VAR10'},
              {:LOOPVAR1=>'LOOP3-VAR1',:LOOPVAR2=>'LOOP3-VAR2',:LOOPVAR3=>'LOOP3-VAR3',:LOOPVAR10=>'LOOP3-VAR10'},
              {:LOOPVAR1=>'LOOP4-VAR1',:LOOPVAR2=>'LOOP4-VAR2',:LOOPVAR3=>'LOOP4-VAR3',:LOOPVAR10=>'LOOP4-VAR10'},
             ]
}

VAR1, VAR2, VAR3, VAR10 = %w{VAR1 VAR2 VAR3 VAR10}

# -------------------------

def test_tmpl(file, *args)
  params = args.inject({}) do |result, item|
    result.update(item)
  end
  testname = file
  test_tmpl_complete(testname, params)
  test_tmpl_output(testname, params)
end

def test_tmpl_output(testname, params)
  Dir.chdir 'templates-Pro'
  file = testname  
  count = 1000

  tmplo = HTML::Template::Pro.new(:filename => file + '.tmpl',
                                  :die_on_bad_params => false,
                                  :strict => false,
                                  :case_sensitive => false,
                                  :loop_context_vars => true
                                  )
  tmplo.param(params);

  File.open('/dev/null', 'w') do |file|
    puts "Template::Pro " + '-' * 40
    puts Benchmark::CAPTION
    puts Benchmark.measure{
      count.times do
        tmplo.output(:print_to => file)
      end
    }
  end

  erb = ERB.new(File.read(file + '.erb'))
  File.open('/dev/null', 'w') do |file|
    puts "ERB " + '-' * 40
    puts Benchmark::CAPTION
    puts Benchmark.measure{
      count.times do
        file.puts erb.result(binding)
      end
    }
  end

  erubis = Erubis::FastEruby.new(File.read(file + '.erb'))
  File.open('/dev/null', 'w') do |file|
    puts "Erubis " + '-' * 40
    puts Benchmark::CAPTION
    puts Benchmark.measure{
      count.times do
        file.puts erubis.result(binding)
      end
    }
  end

  Dir.chdir '..'
end

def test_tmpl_complete(testname, params)
  Dir.chdir 'templates-Pro';
  file = testname
  count = 1000

  File.open('/dev/null', 'w') do |file|
    puts "Template::Pro " + '-' * 40
    puts Benchmark::CAPTION
    puts Benchmark.measure{
      count.times do
	tmpl = HTML::Template::Pro.new(:filename => testname + '.tmpl',
                                       :loop_context_vars => true,
                                       :case_sensitive => false,
                                       :die_on_bad_params => false)
	tmpl.param(params)
	tmpl.output(:print_to => file)
      end
    }
  end

  File.open('/dev/null', 'w') do |file|
    puts "ERB " + '-' * 40
    puts Benchmark::CAPTION
    puts Benchmark.measure{
      count.times do
        erb = ERB.new(File.read(testname + '.erb'))
        file.puts erb.result(binding)
      end
    }
  end

  File.open('/dev/null', 'w') do |file|
    puts "Erubis " + '-' * 40
    puts Benchmark::CAPTION
    puts Benchmark.measure{
      count.times do
        erubis = Erubis::FastEruby.new(File.read(testname + '.erb'))
        file.puts erubis.result(binding)
      end
    }
  end

  Dir.chdir '..'
end

test_tmpl('test_var1', varset1);
test_tmpl('test_var2', varset1);

__END__
test_tmpl('test_var3', varset1, varset2);
test_tmpl('test_if1',  varset1);
test_tmpl('test_if2',  varset1);
test_tmpl('test_if3',  refset1);
test_tmpl('test_loop1', varset1, refset1);
test_tmpl('test_loop2', varset1, refset1);
test_tmpl('test_loop3', varset1, refset1);
test_tmpl('test_loop4', varset1, refset1);
test_tmpl('test_loop5', varset1, refset1);
