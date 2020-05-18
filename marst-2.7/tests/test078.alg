begin

    comment library 1;

    integer
        sum;
    procedure p1(s, r, i, b, ra, ia);
        string
            s;
        real
            r;
        integer
            i;
        Boolean
            b;
        real array
            ra;
        integer array
            ia;
        sum := sum + r + i + (if b then 1 else 0) + ra[1] +
            ia[1];

    procedure p2(ba, l, sw, pr, rpr, ipr, bpr);
        Boolean array
            ba;
        label
            l;
        switch
            sw;
        procedure
            pr;
        real procedure
            rpr;
        integer procedure
            ipr;
        Boolean procedure
            bpr;
        begin
        pr;
        sum := sum + rpr + ipr;
        if bpr then
            go to l;
        if ba[1] then
            go to sw[2]
        end;

    procedure p3(r, i, b, ra, ia, ba, l);
        value
            r,
            i,
            b,
            ra,
            ia,
            ba,
            l;
        real
            r;
        integer
            i;
        Boolean
            b;
        real array
            ra;
        integer array
            ia;
        Boolean array
            ba;
        label
            l;
        begin
        sum := sum + r + i + ra[1] + ia[1] + (if b then 1
            else 0) + (if ba[1] then 1 else 0);
        if ba[1] then
            go to l
        end;

    real
        r;
    Boolean
        b;
    real array
        ra[1 : 1];
    integer array
        ia[1 : 1];
    Boolean array
        ba[1 : 1];
    integer procedure ipr;
        ipr := 2 * sum;

    real procedure rpr;
        rpr := 3 * sum;

    Boolean procedure bpr;
        bpr := ! b;

    procedure pr;
        sum := 5 * sum + 1;

    switch ss :=
        work,
        fail;
    sum := 0;
    ia[1] := - 2;
    ra[1] := 3.55;
    ba[1] := false;
    b := true;
    r := 6.4;
    p1(ra, "ab", "ab", "ab", "ab", "ab" );
    p1(ss, ra, ra, ra, ss, ss);
    p1(pr, ss, ss, ss, pr, pr);
    p1(ss[2], pr, pr, pr, ss[2], ss[2]);
    p1(true, ss[2], ss[2], ss[2], true, true);
    p1(r, false, false, r, r, r);
    p2( "ab", "ab", "ab", "ab", "ab", "ab", "ab" );
    p2(ra, ra, ra, ra, ra, ra, ra);
    p2(ss, ss, ss[2], ss, ss, ss, ss);
    p2(pr, pr, pr, r, pr, pr, pr);
    p2(fail, ss, fail, fail, fail, fail, fail);
    p2(true, true, true, true, true, true, true);
    p2(r, r, r, r, r, r, r);
    p3( "ab", "ab", "ab", "ab", "ab", "ab", "ab" );
    p3(ra, ra, ra, r, r, r, ra);
    p3(ss, ss, ss, ss, ss, ss, ss);
    p3(pr, pr, pr, pr, pr, pr, pr);
    p3(fail, fail, fail, fail, fail, fail, ss);
    p3(true, true, r, true, true, true, true);
    p3(true, true, r, r, r, r, r);
    work :;
    if false then
        fail : outreal(1, - 1);
end
