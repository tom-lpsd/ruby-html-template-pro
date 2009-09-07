use Test;
BEGIN {
    $tests=30;
    plan tests => $tests;
}
use HTML::Template::Pro;
use vars qw/$test/;
my $t = HTML::Template::Pro->new( filename => 'templates-Pro/test_malloc.tmpl' , debug=>0);
for($x=250;$x<250+$tests;$x++) {
    my $txt='x'x$x;
    $t->param('text' => $txt );
    ok($t->output eq ($txt . "\n")) ;
}
