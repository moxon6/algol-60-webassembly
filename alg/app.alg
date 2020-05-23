begin
   integer array tetrisBoard[0:17,0:9];
   integer array tetronimos[0:6,0:3, 0:3];
   integer array currentTetronimo[0:3, 0:3];
   integer currentPositionX;
   integer currentPositionY;

   procedure printMatrix(channel, a, dim1, dim2);
   integer channel; integer array a; integer dim1; integer dim2;
   
   begin
      integer i, j; integer input;
      for i := 0 step 1 until dim1-1 do
      begin
         for j := 0 step 1 until dim2-1 do outinteger (channel, a[i,j]);
         outstring(channel,"\n");
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

   procedure initialiseTetronimo(tetronimoIndex, color);
   integer tetronimoIndex; integer color;
   begin
      integer i, j;
      for i := 0 step 1 until 3 do
      begin
         for j := 0 step 1 until 3 do
            currentTetronimo[i,j] := tetronimos[tetronimoIndex,i,j] * color;
      end
   end;

   procedure placeCurrentTetronimo;
   begin
      integer i, j;
      for i := 0 step 1 until 3 do
      begin
         for j := 0 step 1 until 3 do
            tetrisBoard[i + currentPositionY, j + currentPositionX] := currentTetronimo[i,j];
      end;
      outstring(1, "\n");
   end;

   procedure copyTetronimo(src, dest);
   integer array src; integer array dest;
   begin
      integer i, j;
      for i := 0 step 1 until 3 do
         begin
            for j := 0 step 1 until 3 do
               dest[i,j] := src[i,j];
      end
   end;

   procedure transposeTetronimo(src, dest);
   integer array src; integer array dest;
   begin
      integer i, j;
      for i := 0 step 1 until 3 do
         begin
            for j := 0 step 1 until 3 do
               dest[i,j] := src[j,i];
      end
   end;

   comment "I Tetronimo";
   tetronimos[0,0,0] := 1; tetronimos[0,0,1] := 1; tetronimos[0,0,2] := 1; tetronimos[0,0,3] := 1;
   tetronimos[0,1,0] := 0; tetronimos[0,1,1] := 0; tetronimos[0,1,2] := 0; tetronimos[0,1,3] := 0;
   tetronimos[0,2,0] := 0; tetronimos[0,2,1] := 0; tetronimos[0,2,2] := 0; tetronimos[0,2,3] := 0;
   tetronimos[0,3,0] := 0; tetronimos[0,3,1] := 0; tetronimos[0,3,2] := 0; tetronimos[0,3,3] := 0;

   comment "O Tetronimo";
   tetronimos[1,0,0] := 0; tetronimos[1,0,1] := 1; tetronimos[1,0,2] := 1; tetronimos[1,0,3] := 0;
   tetronimos[1,1,0] := 0; tetronimos[1,1,1] := 1; tetronimos[1,1,2] := 1; tetronimos[1,1,3] := 0;
   tetronimos[1,2,0] := 0; tetronimos[1,2,1] := 0; tetronimos[1,2,2] := 0; tetronimos[1,2,3] := 0;
   tetronimos[1,3,0] := 0; tetronimos[1,3,1] := 0; tetronimos[1,3,2] := 0; tetronimos[1,3,3] := 0;

   comment "T Tetronimo";
   tetronimos[2,0,0] := 0; tetronimos[2,0,1] := 1; tetronimos[2,0,2] := 0; tetronimos[2,0,3] := 0;
   tetronimos[2,1,0] := 1; tetronimos[2,1,1] := 1; tetronimos[2,1,2] := 1; tetronimos[2,1,3] := 0;
   tetronimos[2,2,0] := 0; tetronimos[2,2,1] := 0; tetronimos[2,2,2] := 0; tetronimos[2,2,3] := 0;
   tetronimos[2,3,0] := 0; tetronimos[2,3,1] := 0; tetronimos[2,3,2] := 0; tetronimos[2,3,3] := 0;

   comment "S Tetronimo";
   tetronimos[3,0,0] := 0; tetronimos[3,0,1] := 1; tetronimos[3,0,2] := 1; tetronimos[3,0,3] := 0;
   tetronimos[3,1,0] := 1; tetronimos[3,1,1] := 1; tetronimos[3,1,2] := 0; tetronimos[3,1,3] := 0;
   tetronimos[3,2,0] := 0; tetronimos[3,2,1] := 0; tetronimos[3,2,2] := 0; tetronimos[3,2,3] := 0;
   tetronimos[3,3,0] := 0; tetronimos[3,3,1] := 0; tetronimos[3,3,2] := 0; tetronimos[3,3,3] := 0;

   comment "Z Tetronimo";
   tetronimos[4,0,0] := 1; tetronimos[4,0,1] := 1; tetronimos[4,0,2] := 0; tetronimos[4,0,3] := 0;
   tetronimos[4,1,0] := 0; tetronimos[4,1,1] := 1; tetronimos[4,1,2] := 1; tetronimos[4,1,3] := 0;
   tetronimos[4,2,0] := 0; tetronimos[4,2,1] := 0; tetronimos[4,2,2] := 0; tetronimos[4,2,3] := 0;
   tetronimos[4,3,0] := 0; tetronimos[4,3,1] := 0; tetronimos[4,3,2] := 0; tetronimos[4,3,3] := 0;

   comment "J Tetronimo";
   tetronimos[5,0,0] := 1; tetronimos[5,0,1] := 0; tetronimos[5,0,2] := 0; tetronimos[5,0,3] := 0;
   tetronimos[5,1,0] := 1; tetronimos[5,1,1] := 1; tetronimos[5,1,2] := 1; tetronimos[5,1,3] := 0;
   tetronimos[5,2,0] := 0; tetronimos[5,2,1] := 0; tetronimos[5,2,2] := 0; tetronimos[5,2,3] := 0;
   tetronimos[5,3,0] := 0; tetronimos[5,3,1] := 0; tetronimos[5,3,2] := 0; tetronimos[5,3,3] := 0;

   comment "L Tetronimo";
   tetronimos[6,0,0] := 0; tetronimos[6,0,1] := 0; tetronimos[6,0,2] := 1; tetronimos[6,0,3] := 0;
   tetronimos[6,1,0] := 1; tetronimos[6,1,1] := 1; tetronimos[6,1,2] := 1; tetronimos[6,1,3] := 0;
   tetronimos[6,2,0] := 0; tetronimos[6,2,1] := 0; tetronimos[6,2,2] := 0; tetronimos[6,2,3] := 0;
   tetronimos[6,3,0] := 0; tetronimos[6,3,1] := 0; tetronimos[6,3,2] := 0; tetronimos[6,3,3] := 0;

   begin
      
         integer i;
         integer currentKey;
         boolean done;
         integer array currentTetronimoBuffer[0:3, 0:3];
         
         done := false;
         
         inline("startup();");

         for i:=0 while !done do
         begin                        
            if (currentKey = 50) then begin
               done := true;
            end;
            outstring(1, "CALL:setElementProperty(\"#tetris-row-0-cell-0\", \"style.backgroundColor\", \"red\")\n");
            inline("emscripten_sleep(1000);");
         end;

         

         currentPositionX := 5;
         currentPositionY := 6;
         
         for i := 0 step 1 until 5 do
         begin
            initialiseTetronimo(i, 5);
            printMatrix(1, currentTetronimo, 4,4);
            outstring(1, "\n")
         end;

         transposeTetronimo(currentTetronimo, currentTetronimoBuffer);
         copyTetronimo(currentTetronimoBuffer, currentTetronimo);

         placeCurrentTetronimo;
         printMatrix(1, tetrisBoard, 18,10);
         inline("emscripten_sleep(250);");
      end
   end
