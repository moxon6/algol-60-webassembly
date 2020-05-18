begin
    comment a syntax pass is required to remove comments;
    procedure p( st );comment st is a string;comment;
        string st; ;comment dummy body;
    p( ";comment\";\"" );
    begin
        procedure q( a ) parameter comment :( b );
            real a, b; ;comment dummy again;
        q( 1.0, 2.5 ); comment is the same as;
        q( 1.0) with parameter comment:( 2.5 );
    end comment can include most basic symbols for
        while then do if := begin( which are not matched;
end
