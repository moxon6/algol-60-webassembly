real procedure cputime;
inline("my_dsa.retval.u.real_val = (double)clock() / (double)CLOCKS_PER_SEC;");

begin
comment library 3;


integer i,cl,n,rep;

array r,a,b[1:30],c[1:10];

real j,root,x;

integer procedure even(x);
    value x;
    integer x;
    even:=if x%2*2 = x then 1 else -1;


procedure gamm(r,a,b,c);
    array r,a,b,c;
    begin    integer rep,i,cl;
        real j,root,x;
        rep:=100*n;
        outreal(1, cputime);
        for cl:= 1 step 1 until rep do
            ;
        outreal(1, cputime);
        rep:=n;
        outreal(1, cputime);
        for cl:=1 step 1 until rep do
        for i:= 1 step 1 until 30 do
            r[i]:=a[i]+b[i];
        outreal(1, cputime);
        for cl:=1 step 1 until rep do
        for i:=1 step 1 until 30 do
            r[i]:=a[i]*b[i];
        outreal(1, cputime);
        rep:=6*n;
        x := .5;
        outreal(1, cputime);
        for cl:= 1 step 1 until rep do
        begin    j:=0.0;
            for i:= 1 step 1 until 10 do
                j:=(j+c[i])*x
        end;
        outreal(1, cputime);
        outreal(1, j);
        outreal(1, cputime);
        for cl:= 1 step 1 until rep do
        begin    j:=c[1];
            for i:=2 step 1 until 10 do
                if c[i]>j then j:=c[i]
        end;
        outreal(1, cputime);
        j:=2;
        rep := 8*n;
        outreal(1, cputime);
        for cl:=1 step 1 until rep do
        begin    root:=1.0;
            for i:=1 step 1 until 5 do
                root:= 0.5*(root+j/root)
        end;
        outreal(1, cputime);
        outreal(1, root);
    end of gamm;


    i:=1;
    for j:=104.5,3.7,-68.9,228.2,-1004.4,500.0,308.1,45.0,152.35,161.8 do
    begin    c[i]:=j;
        i:=i+1
    end;
    for i:=1 step 1 until 30 do
    begin    a[i]:=even(i)*i*i;
        b[i]:=1-even(i)*i*i
    end;
    n := 2000;
    rep:=100*n;
    outreal(1, cputime);
    for cl:= 1 step 1 until rep do
        ;
    outreal(1, cputime);
    rep:=n;
    outreal(1, cputime);
    for cl:=1 step 1 until rep do
    for i:= 1 step 1 until 30 do
        r[i]:=a[i]+b[i];
    outreal(1, cputime);
    for cl:=1 step 1 until rep do
    for i:=1 step 1 until 30 do
        r[i]:=a[i]*b[i];
    outreal(1, cputime);
    rep:=6*n;
        x := .5;
    outreal(1, cputime);
    for cl:= 1 step 1 until rep do
    begin    j:=0.0;
        for i:= 1 step 1 until 10 do
            j:=(j+c[i])*x
    end;
    outreal(1, cputime);
    outreal(1, j);
    outreal(1, cputime);
    for cl:= 1 step 1 until rep do
    begin    j:=c[1];
        for i:=2 step 1 until 10 do
            if c[i]>j then j:=c[i]
    end;
    outreal(1, cputime);
    j:=2;
    rep := 8*n;
    outreal(1, cputime);
    for cl:=1 step 1 until rep do
    begin    root:=1.0;
        for i:=1 step 1 until 5 do
            root:= 0.5*(root+j/root)
    end;
    outreal(1, cputime);
    outreal(1, root);
    gamm(r,a,b,c);

end
