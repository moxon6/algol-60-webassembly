begin
   
   integer array lockedPiecesBoard[0:17,0:9];
   
   comment "Allocate array to store what will be actually displayed - Recalculate on each loop";
   integer array compositeBoard[0:17,0:9]; 
   
   integer array tetronimos[0:6,0:3, 0:1];
   integer array currentTetronimo[0:3, 0:1];
   
   integer currentPositionX;
   integer currentPositionY;

   procedure printMatrix(channel, a, dim1, dim2);
   integer channel; integer array a; integer dim1; integer dim2;
   
   begin
      integer i, j; integer input;
      for i := 0 step 1 until dim1-1 do
      begin
         for j := 0 step 1 until dim2-1 do 
            if a[i,j] > 0 then
               setSquareColor(i,j, "blue")               
            else
               setSquareColor(i,j, "green");
      end
   end;

   procedure rotateMatrix(matrix, dimension);
   integer array matrix; integer dimension;
   begin
      integer i, j;
      for i := 0 step 1 until dimension-1 do
      begin
         for j := 0 step 1 until dimension-1 do
            outstring(1, "\n");
      end
   end;

   procedure initialiseTetronimo(tetronimoIndex);
   integer tetronimoIndex;
   begin
      integer i, j;
      for i := 0 step 1 until 3 do
      begin
            currentTetronimo[i,0] := tetronimos[tetronimoIndex,i,0];
            currentTetronimo[i,1] := tetronimos[tetronimoIndex,i,1];
      end
   end;

   procedure placeCurrentTetronimo;
   begin
      integer i, j, k;

      comment "Copy locked pieces into composite board";
      for i := 0 step 1 until 17 do
         begin
            for j := 0 step 1 until 9 do
               compositeBoard[i,j] := lockedPiecesBoard[i,j];
      end

      comment "Insert current tetronimo into current board";
      for k := 0 step 1 until 3 do
      begin
         compositeBoard[
            currentTetronimo[k,1] + currentPositionY,
            currentTetronimo[k,0] + currentPositionX
         ] := 1;
      end;
   end;

   procedure transposeTetronimo(src, dest);
   integer array src; integer array dest;
   begin
      integer i, j;
      for i := 0 step 1 until 3 do
         begin
            for j := 0 step 1 until 1 do
               dest[i,j] := src[j,i];
      end
   end;

   procedure setSquareColor(x,y,color);
   integer x,y; string color;
   begin
      outstring(1, "CALL:setElementProperty('#tetris-row-");
      outinteger(1, x);
      outstring(1, "-cell-");
      outinteger(1, y);
      outstring(1, "', 'style.backgroundColor','");
      outstring(1, color);
      outstring(1, "')\n");
   end;

   comment "First index corresponds to tetronimo";
   comment "Each line has two values corresponding to a single (x,y) pair";

   comment "I Tetronimo";
   tetronimos[0,0,0] := 0; tetronimos[0,0,1] := 0; comment "Square 1";
   tetronimos[0,1,0] := 1; tetronimos[0,1,1] := 0; comment "Square 2";
   tetronimos[0,2,0] := 2; tetronimos[0,2,1] := 0; comment "Square 3";
   tetronimos[0,3,0] := 3; tetronimos[0,3,1] := 0; comment "Square 4";

   comment "O Tetronimo";
   tetronimos[1,0,0] := 0; tetronimos[1,0,1] := 0; comment "Square 1";
   tetronimos[1,1,0] := 1; tetronimos[1,1,1] := 0; comment "Square 2";
   tetronimos[1,2,0] := 0; tetronimos[1,2,1] := 1; comment "Square 3";
   tetronimos[1,3,0] := 1; tetronimos[1,3,1] := 1; comment "Square 4";

   comment "T Tetronimo";
   tetronimos[2,0,0] := 0; tetronimos[2,0,1] := 0; comment "Square 1";
   tetronimos[2,1,0] := 1; tetronimos[2,1,1] := 0; comment "Square 2";
   tetronimos[2,2,0] := 2; tetronimos[2,2,1] := 0; comment "Square 3";
   tetronimos[2,3,0] := 1; tetronimos[2,3,1] := 1; comment "Square 4";

   comment "J Tetronimo";
   tetronimos[3,0,0] := 1; tetronimos[3,0,1] := 0; comment "Square 1";
   tetronimos[3,1,0] := 1; tetronimos[3,1,1] := 1; comment "Square 2";
   tetronimos[3,2,0] := 0; tetronimos[3,2,1] := 2; comment "Square 3";
   tetronimos[3,3,0] := 1; tetronimos[3,3,1] := 2; comment "Square 4";

   comment "J Tetronimo";
   tetronimos[4,0,0] := 0; tetronimos[4,0,1] := 0; comment "Square 1";
   tetronimos[4,1,0] := 0; tetronimos[4,1,1] := 1; comment "Square 2";
   tetronimos[4,2,0] := 0; tetronimos[4,2,1] := 2; comment "Square 3";
   tetronimos[4,3,0] := 1; tetronimos[4,3,1] := 2; comment "Square 4";

   comment "S Tetronimo";
   tetronimos[5,0,0] := 1; tetronimos[5,0,1] := 0; comment "Square 1";
   tetronimos[5,1,0] := 2; tetronimos[5,1,1] := 0; comment "Square 2";
   tetronimos[5,2,0] := 0; tetronimos[5,2,1] := 1; comment "Square 3";
   tetronimos[5,3,0] := 1; tetronimos[5,3,1] := 1; comment "Square 4";

   comment "Z Tetronimo";
   tetronimos[6,0,0] := 0; tetronimos[6,0,1] := 0; comment "Square 1";
   tetronimos[6,1,0] := 1; tetronimos[6,1,1] := 0; comment "Square 2";
   tetronimos[6,2,0] := 1; tetronimos[6,2,1] := 1; comment "Square 3";
   tetronimos[6,3,0] := 2; tetronimos[6,3,1] := 1; comment "Square 4";

   begin
      integer i;
      integer currentKey;
      boolean done;
      
      done := false;
      inline("startup();");

      currentPositionX := 5;
      currentPositionY := 6;

      for i:=0 while !done do
         begin                        
            if (currentKey = 50) then begin
               done := true;
            end;
            
            initialiseTetronimo(6);

            placeCurrentTetronimo;
            printMatrix(1, compositeBoard, 18,10);
            inline("emscripten_sleep(500);");
            currentPositionY := currentPositionY + 1;
         end;
   end;
end;

