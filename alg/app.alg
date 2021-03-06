begin
   
   integer array lockedPiecesBoard[0:17,0:9];
   
   comment "Allocate array to store what will be actually displayed - Recalculate on each loop";
   integer array compositeBoard[0:17,0:9]; 
   
   integer array tetronimos[0:6,0:3, 0:1];

   comment "Rotation gives xNew = -y + (dx + dy) and YNew =x + (dy - dx)";
   comment "Where dy, dy are center of piece";
   comment "dx,dy can be non-integral, but dx+dy and dy-dx are not, so these are stored instead";
   integer array tetronimosOffset[0:6, 0:1];

   comment "Random seed populated externally for LCG randInt";
   integer seed;
   integer score;

   integer array currentTetronimo[0:3, 0:1];
   
   integer currentPositionX;
   integer currentPositionY;

   procedure randInt(max, out);
   comment "Using random0 LCG";
   integer max, out;
   begin
      seed := seed * 8121 + 28411; 
      modulo(seed, 134456);
      out := seed;
      modulo(out, max);
   end;

   procedure drawBoard(board);
   integer array board;
   begin
      integer x,y;
      for y := 0 step 1 until 17 do
      begin
         for x := 0 step 1 until 9 do 
            setSquareColor(y,x, board[y,x]);
      end;
      comment "We queue up draw commands and execute in batch for performance";
      outstring(1, "EXECUTE\n");
   end;

   procedure initialiseTetronimo(tetronimoIndex);
   integer tetronimoIndex;
   begin
      integer i;
      for i := 0 step 1 until 3 do
      begin
            currentTetronimo[i,0] := tetronimos[tetronimoIndex,i,0];
            currentTetronimo[i,1] := tetronimos[tetronimoIndex,i,1];
      end
   end;

   procedure placeCurrentTetronimo(currentColorIndex);
   integer currentColorIndex;
   begin
      integer x,y,squareIndex;

      comment "Copy locked pieces into composite board";
      for y := 0 step 1 until 17 do
         begin
            for x := 0 step 1 until 9 do
               compositeBoard[y,x] := lockedPiecesBoard[y,x];
      end

      comment "Insert current tetronimo into current board";
      for squareIndex := 0 step 1 until 3 do
      begin
         compositeBoard[
            currentTetronimo[squareIndex,1] + currentPositionY,
            currentTetronimo[squareIndex,0] + currentPositionX
         ] := currentColorIndex;
      end;
   end;

   procedure lockInCurrentPiece;
   begin
      integer x,y;
      comment "Lock current visible board in";
      for y := 0 step 1 until 17 do
         begin
            for x := 0 step 1 until 9 do
               lockedPiecesBoard[y,x] := compositeBoard[y,x];
      end
   end;

   procedure clearLine(rowIndex);
   integer rowIndex;
   begin
      integer x,y;
      for y := rowIndex step -1 until 1 do
         begin
            for x := 0 step 1 until 9 do 
               lockedPiecesBoard[y,x] := lockedPiecesBoard[y-1,x];
         end
   end;

   procedure clearFullLines;
   begin
      integer x,y;
      for y := 0 step 1 until 17 do
         begin
            boolean fullLine;
            fullLine := true;
            for x := 0 step 1 until 9 do fullLine := fullLine & lockedPiecesBoard[y,x] > 0;
            if (fullLine) then 
               begin
                  score := score + 100;
                  clearLine(y);
               end;
         end
   end;

   procedure setSquareColor(x,y,colorIndex);
   integer x,y,colorIndex;
   begin
      procedure printColorByIndex(colorIndex);
      integer colorIndex;
      begin
         if (colorIndex = 1) then outstring(1, "aqua")
         else if (colorIndex = 2) then outstring(1, "blue")
         else if (colorIndex = 3) then outstring(1, "orange")
         else if (colorIndex = 4) then outstring(1, "yellow")
         else if (colorIndex = 5) then outstring(1, "green")
         else if (colorIndex = 6) then outstring(1, "purple")
         else if (colorIndex = 7) then outstring(1, "red")
         else outstring(1, "transparent");
      end;

      outstring(1, "CALL:setElementProperty('#tetris-row-");
      outinteger(1, x);
      outstring(1, "-cell-");
      outinteger(1, y);
      outstring(1, "', 'className','");
      outstring(1, "tetris-cell-");
      printColorByIndex(colorIndex);
      outstring(1, "')\n");
   end;

   procedure canMove(dx, dy, out);
   boolean out; integer dx,dy;
   begin
      integer squareIndex;
      out := true;
      for squareIndex := 0 step 1 until 3 do
      begin
         boolean canPlaceResult;
         canPlace(
            currentTetronimo[squareIndex,0] + currentPositionX + dx,
            currentTetronimo[squareIndex,1] + currentPositionY + dy,
            canPlaceResult
         );
         out := out & canPlaceResult;
      end
   end;

   procedure tryMove(dx, dy);
   integer dx, dy;
   begin
         boolean canMoveResult;
         canMove(dx, dy, canMoveResult);
         if (canMoveResult) then 
         begin
            currentPositionX := currentPositionX + dx;
            currentPositionY := currentPositionY + dy;
         end;
   end;

   procedure tryRotate(currentPieceIndex);
   integer currentPieceIndex;
   begin
      integer squareIndex;
      boolean canRotateResult;
      canRotateResult := true;
      canRotate(currentPieceIndex, canRotateResult);
      if canRotateResult then
         begin
            for squareIndex := 0 step 1 until 3 do
               begin      
                  integer x,y;
                  x := currentTetronimo[squareIndex,0];
                  y := currentTetronimo[squareIndex,1];
                  currentTetronimo[squareIndex,0] := - y + tetronimosOffset[currentPieceIndex,0];
                  currentTetronimo[squareIndex,1] :=   x + tetronimosOffset[currentPieceIndex,1];
               end;
         end;
         
   end;

   procedure canRotate(currentPieceIndex, out);
   boolean out; integer currentPieceIndex;
   begin
      integer squareIndex;
      for squareIndex := 0 step 1 until 3 do
      begin
            integer x,y;
            boolean canPlaceResult;

            x := - currentTetronimo[squareIndex,1] + tetronimosOffset[currentPieceIndex,0] + currentPositionX;
            y :=   currentTetronimo[squareIndex,0] + tetronimosOffset[currentPieceIndex,1] + currentPositionY;

            canPlace(x, y, canPlaceResult);
            out := out & canPlaceResult;

      end
   end;

   procedure canPlace(x,y, out);
   integer x, y; boolean out;
   begin
      out := (
         y > -1 &
         y < 18 &
         x > -1 &
         x < 10
      );
      if out then out := out & lockedPiecesBoard[y, x] = 0;
   end;

   procedure modulo(a,b);
   integer a, b;
   begin
      integer floor;
      floor := (a / b);
      comment "Enforce floor division (ceiling by default)";
      if (floor * b > a) then floor := floor - 1;
      a := a - (b * floor);
   end;

   procedure increment(out);
   integer out;
   out := out + 1;

   procedure drawScore;
   begin
      outstring(1, "CALL:setElementProperty('#score-value', 'innerHTML', '");
      outinteger(1, score);
      outstring(1, "')\n");
      outstring(1, "EXECUTE\n");
   end

   comment "First index corresponds to tetronimo";
   comment "Each line has two values corresponding to a single (x,y) pair";

   comment "I Tetronimo";
   tetronimos[0,0,0] := 0; tetronimos[0,0,1] := 0; comment "Square 1";
   tetronimos[0,1,0] := 1; tetronimos[0,1,1] := 0; comment "Square 2";
   tetronimos[0,2,0] := 2; tetronimos[0,2,1] := 0; comment "Square 3";
   tetronimos[0,3,0] := 3; tetronimos[0,3,1] := 0; comment "Square 4";

   tetronimosOffset[0,0] := 2; tetronimosOffset[0,1] := -1; 
   
   comment "O Tetronimo";
   tetronimos[1,0,0] := 0; tetronimos[1,0,1] := 0; comment "Square 1";
   tetronimos[1,1,0] := 1; tetronimos[1,1,1] := 0; comment "Square 2";
   tetronimos[1,2,0] := 0; tetronimos[1,2,1] := 1; comment "Square 3";
   tetronimos[1,3,0] := 1; tetronimos[1,3,1] := 1; comment "Square 4";

   tetronimosOffset[1,0] := 1; tetronimosOffset[1,1] := 0; 

   comment "T Tetronimo";
   tetronimos[2,0,0] := 0; tetronimos[2,0,1] := 0; comment "Square 1";
   tetronimos[2,1,0] := 1; tetronimos[2,1,1] := 0; comment "Square 2";
   tetronimos[2,2,0] := 2; tetronimos[2,2,1] := 0; comment "Square 3";
   tetronimos[2,3,0] := 1; tetronimos[2,3,1] := 1; comment "Square 4";

   tetronimosOffset[2,0] := 1; tetronimosOffset[2,1] := -1;

   comment "J Tetronimo";
   tetronimos[3,0,0] := 1; tetronimos[3,0,1] := 0; comment "Square 1";
   tetronimos[3,1,0] := 1; tetronimos[3,1,1] := 1; comment "Square 2";
   tetronimos[3,2,0] := 0; tetronimos[3,2,1] := 2; comment "Square 3";
   tetronimos[3,3,0] := 1; tetronimos[3,3,1] := 2; comment "Square 4";

   tetronimosOffset[3,0] := 2; tetronimosOffset[3,1] := 0;

   comment "L Tetronimo";
   tetronimos[4,0,0] := 0; tetronimos[4,0,1] := 0; comment "Square 1";
   tetronimos[4,1,0] := 0; tetronimos[4,1,1] := 1; comment "Square 2";
   tetronimos[4,2,0] := 0; tetronimos[4,2,1] := 2; comment "Square 3";
   tetronimos[4,3,0] := 1; tetronimos[4,3,1] := 2; comment "Square 4";

   tetronimosOffset[4,0] := 1; tetronimosOffset[4,1] := 1;

   comment "S Tetronimo";
   tetronimos[5,0,0] := 1; tetronimos[5,0,1] := 0; comment "Square 1";
   tetronimos[5,1,0] := 2; tetronimos[5,1,1] := 0; comment "Square 2";
   tetronimos[5,2,0] := 0; tetronimos[5,2,1] := 1; comment "Square 3";
   tetronimos[5,3,0] := 1; tetronimos[5,3,1] := 1; comment "Square 4";

   tetronimosOffset[5,0] := 2; tetronimosOffset[5,1] := 0;

   comment "Z Tetronimo";
   tetronimos[6,0,0] := 0; tetronimos[6,0,1] := 0; comment "Square 1";
   tetronimos[6,1,0] := 1; tetronimos[6,1,1] := 0; comment "Square 2";
   tetronimos[6,2,0] := 1; tetronimos[6,2,1] := 1; comment "Square 3";
   tetronimos[6,3,0] := 2; tetronimos[6,3,1] := 1; comment "Square 4";

   tetronimosOffset[6,0] := 2; tetronimosOffset[6,1] := 0;

   ininteger(0, seed);

   begin
      integer i;
      integer currentKey;
      boolean done;
      integer currentFrame;
      integer currentColorIndex;
      integer currentPieceIndex;
      integer rotationDelay; comment "Delay to prevent over-eager rotation";
      
      inline("startup();");
      
      currentFrame := 0;
      rotationDelay:= 0;
      done := false;
      currentPositionX := 4;
      currentPositionY := 0;
      score := 0;
      
      randInt(7, currentPieceIndex);
      
      randInt(7, currentColorIndex);
      increment(currentColorIndex);
      
      initialiseTetronimo(currentPieceIndex);
      
      
      for i := 0 while currentKey = 0 do begin
         ininteger(0, currentKey);
         inline("emscripten_sleep(10);");
      end;

      outstring(1, "CALL:setElementProperty('#press-key-message', 'style.display', 'none')\n");
      outstring(1, "CALL:document.querySelector('#audio-theme').play()\n");
      outstring(1, "EXECUTE\n");
      

      for i := 0 while !done do
         begin
            currentFrame := currentFrame + 1;
            modulo(currentFrame, 10);

            if (rotationDelay > 0) then rotationDelay := rotationDelay - 1;

            ininteger(0, currentKey);
            if (currentKey = 81) then
               done := true;
            if (currentKey = 65) then
               tryMove(-1,0);
            if (currentKey = 68) then
               tryMove(1,0);
            
            if (currentKey = 83) then
               begin
                  boolean canMoveResult;
                  tryMove(0,1);
                  canMove(0,1,canMoveResult);
                  if canMoveResult then currentFrame := 1;
               end;
            
            if (currentKey = 87 & rotationDelay = 0) then
               begin
                  tryRotate(currentPieceIndex);
                  rotationDelay := 4;
               end;

            placeCurrentTetronimo(currentColorIndex);
            drawBoard(compositeBoard);
            drawScore;
            
            if (currentFrame = 0) then 
               begin
                  comment "Auto move down logic";
                  begin
                     boolean canMoveDownResult;
                     canMove(0, 1, canMoveDownResult);
                     if canMoveDownResult then
                        currentPositionY := currentPositionY + 1
                     else
                        begin
                           lockInCurrentPiece;
                           clearFullLines;
                           begin
                              boolean canPlaceResult;
                              currentPositionX := 4;
                              currentPositionY := 0;

                              randInt(7, currentColorIndex);
                              increment(currentColorIndex);
                              randInt(7, currentPieceIndex);
                              initialiseTetronimo(currentPieceIndex);
                              canMove(0,0,canPlaceResult);

                              if (!canPlaceResult) then 
                              begin
                                 done := true;
                                 outstring(1, "CALL:setElementProperty('#game-over-message', 'style.display', 'block')\n");
                                 outstring(1, "CALL:document.querySelector('#audio-theme').pause()\n");
                                 outstring(1, "EXECUTE\n");
                              end;
                           end
                        end
                        
                  end;
               end;
            
            inline("emscripten_sleep(50);");
            
         end;
   end;
end;