begin

    comment   string tests;

    procedure p(st);
        string
            st;
        ;

    p( "*string string \"nested\"\"\"\"\" begin end;" );
    p( " ,
        " )
end
