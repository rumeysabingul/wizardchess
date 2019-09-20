#include <stdio.h>
#include <math.h>
#include <stdlib.h>

char board[64]={'r','n','b','q','k','b','n','r',
                  'p','p','p','p','p','p','p','p',
                  ' ',' ',' ',' ',' ',' ',' ',' ',
                  ' ',' ',' ',' ',' ',' ',' ',' ',
                  ' ',' ',' ',' ',' ',' ',' ',' ',
                  ' ',' ',' ',' ',' ',' ',' ',' ',
                  'P','P','P','P','P','P','P','P',
                  'R','N','B','Q','K','B','N','R',};


int checkStatus = 0;



void setup() {
  
  Serial.begin(9600); //opens serial port,sets data rate to 9600 bps

}

void loop() {

  int player = 1; /* 1 white, 0 black */
  char sc,tc;   /* source&target cols */
  int sr,tr;    /* source&target rows */
  int moveStatus = 0;
  char currPlayer;

    if (Serial.available() > 0) 
    {
        Serial.printf("%s player move > ", player ? "White" : "Black");
        
        getPosition(&sc,&sr,&tc,&tr);

         Serial.printf("%c%c to %c%c", sc, sr, tc, tr);
        
        currPlayer = getPlayer(sc,sr);
  
        Serial.println(currPlayer);
        
        if(!isValidCell(sc,sr)) {
            Serial.println("Source position is invalid\n\n");
        }
        else if(!isValidCell(tc,tr)) {
            Serial.println("Target position is invalid\n\n");
        }
        else if((isBlack(&currPlayer) && player) ||
           (isWhite(&currPlayer) && !player)) 
        {
            Serial.println("Illegal piece. \n\n");
            // ısık yak
        }
        else
        {
            moveStatus = makeMove(&sc,&sr,&tc,&tr,&currPlayer);
            switch(moveStatus) 
            {
                case 0:
                    printf("Invalid move!\n\n");
                    break;
                case 1:
                    printf("Your king is in check!\n\n");
                    ++checkStatus;
                    break;
                case 3:
                    printf("Check!\n\n");
                    // Sah diye söylesin
                case 2:
                    player =!player;
                    checkStatus = 0;
                    break;
            }
        }

    }

    if(checkStatus == 2)
    {
        printf("%s player WINS!\n", player ? "Black" : "White");
        delay(10000);
        // reset the board
    }
    
    /*} while(checkStatus < 2); */

}

/*******************************************----------------------------------------------------*/
void getPosition(char *sCol,int *sRow,char *tCol,int *tRow)
{
    char getMoves[4]={'','','',''}; 

    for(int i=0; Serial.available() > 0 ; i++)
    {
        getMoves[i]=Serial.read();
    }

    *sCol=getMoves[0];
    *sRow=getMoves[1];
    *tCol=getMoves[2];
    *tRow=getMoves[3];

   
}

/*Print the initialized board with this function*/
void printBoard()
{
  int i,j,k;

  Serial.print("  a b c d e f g h\n  - - - - - - - -\n");
  
  i=8;
  for(k=0;4*k+7<64;k+=2)
  {
    Serial.printf("%d|", i);
    for(j=4*k;j<=4*k+7;j++)
    {
      Serial.printf("%c", board[j]);
      if(j!=4*k+7)
      {
        Serial.print(" ");
      }
    }
    Serial.print("|\n");
    i--;
  }
  Serial.print("  - - - - - - - -\n");
}


/*This func check the col and row is valid according to board sizes*/
int isValidCell(char col,int row)
{
    if(col>='a'&&col<='h'&&row>=1&&row<=8)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*  This func check the target piece white or black.If the user is black one, */
/*  then target cell can not be black.if its black ,cant move the target cell */
/*                   So returns 0, other wise return 1                        */
int isValidCell2(char *board,char sourceCol,int sourceRow,
                                                   char targetCol,int targetRow)
{
    int sourcepiece,targetpos;

    sourcepiece=(int)((sourceCol-'a')+(8-sourceRow)*8);
    targetpos=(int)((targetCol-'a')+(8-targetRow)*8);


        if(board[sourcepiece]>='a'&&board[sourcepiece]<='z')
        {
            if(board[targetpos]>='a'&&board[targetpos]<='z')
            {
                return 0;
            }
        }
        else if(board[sourcepiece]>='A'&&board[sourcepiece]<='Z')
        {
            if(board[targetpos]>='A'&&board[targetpos]<='Z')
            {
                return 0;
            }
        }
    
    return 1;
}




/*this func finds the current player and return it ------------------------------------------------*/
char getPlayer(char sc,int sr)
{
  int sourcePiece=(int)((sc-'a')+(8-sr)*8);

  if(board[sourcePiece]>='a' && board[sourcePiece]<='z')
  {
    return 'b';
  }
  else if(board[sourcePiece]>='A' && board[sourcePiece]<='Z')
  {
    return 'w';
  }

  return 'f';
}

/*isblack func check the current player is black ----------------------------------------------------*/
int isBlack(char *currPlayer)
{
  if(currPlayer=='b')
    return 1;
    
  return 0;
}
/*iswhite func check the current player is white -----------------------------------------------------*/
int isWhite(char *currPlayer)
{
  if(currPlayer=='w') 
    return 1;
    
  return 0;
}

/*This func make the moves according to game rules -----------------------------------------------------*/
int makeMove(char *sc,int *sr,char *tc, int *tr,char *currentP)
{
  int sourcePos,targetPos;

  int kingCheck,moveCheck,kingCheck2;
  /*Find the index of source and target cells for array using*/
  sourcePos=(int)((*sc-'a')+(8-*sr)*8);
  targetPos=(int)((*tc-'a')+(8-*tr)*8);

  //moveCheck=isPieceMovable(&sc,&sr,&tc,&tr);
  kingCheck=isInCheck();

  /*if move is invalid return 0*/
  if(!moveCheck)
  {
    return 0;
  }
     /*if current player is inCheck cant move anywhere just king 
                               can move, so return 1*/
  else if((currentP=='b' && kingCheck==2 && board[sourcePos]!='k') ||
        (currentP=='w' && kingCheck==1 && board[sourcePos]!='K'))
  {
    return 1;
  }
  /*if current player is incheck and player try to move king*/
  else if((currentP=='b' && kingCheck==2 && board[sourcePos]=='k') ||
        (currentP=='w' && kingCheck==1 && board[sourcePos]=='K'))
  {
    board[targetPos]=board[sourcePos];
    board[sourcePos]=' ';

    kingCheck2=isInCheck();
    /*if the current move is still check dont do it ,return 2*/
    if((currentP=='b' && kingCheck2==2) ||
       (currentP=='w' && kingCheck2==1 ))
    { 
      printf("%d\n", kingCheck2);
      board[sourcePos]=board[targetPos];
      board[targetPos]=' ';
      return 1;
    }

    /*if the last move save the king from check ,return 2*/
    return 2;
    

  }
  
  board[targetPos]=board[sourcePos];
  board[sourcePos]=' ';
  /*if there is no check according to last move and current move was right return 2*/
  if(moveCheck && isInCheck()==0)
  {
    
    return 2;
  }
  
  /*if the last statement doesnt work , change the board like the move 
                                never happened*/
  board[sourcePos]=board[targetPos];
  board[targetPos]=' ';

  /*if there is no check before move happened*/
  if(moveCheck && kingCheck==0)
  {
    board[targetPos]=board[sourcePos];
    board[sourcePos]=' ';

    if((currentP=='b' && isInCheck()==1) ||
       (currentP=='w' && isInCheck()==2))
    {
      return 3;
    }
  }

  return 0;
}

/*This function get source and target position and check them are they movable ---------------------------------*/
int isPieceMovable(char *sc,int *sr,char *tc, int *tr)
{
  int piece,exitt;

  /*Find the index of source cell*/
   piece=(int)((*sc-'a')+(8-*sr)*8);

    switch(board[piece])
    {
      case 'P':
      case 'p':
          exitt=isPawnMovable(sc,sr,tc,tr);
          break;
      case 'R':
      case 'r':
          exitt=isRookMovable(sc,sr,tc,tr);
          break;
      case 'N':
      case 'n':
          exitt=isKnightMovable(sc,sr,tc,tr);
          break;
      case 'B':
      case 'b':
          exitt=isBishopMovable(sc,sr,tc,tr);
          break;
      case 'Q':
      case 'q':
          exitt=isQueenMovable(sc,sr,tc,tr);
          break;
      case 'K':
      case 'k':
          exitt=isKingMovable(sc,sr,tc,tr);
          break;
      default:
    
          return 0;
    }
    return exitt;


    return 0;
}

/*this func take the iskingcheck return value*/
int isInCheck()
{ /*if blackking is in check return 2*/
    if(isKingCheck('k'))
    {
      return 2;
    }
    /*if white is in check return 1*/
    else if(isKingCheck('K'))
    {
      return 1;
    }
  /*Otherwise return 0*/
  return 0;
}

/*This func help the isInCheck func to determine is there a check*/
int isKingCheck(char king)
{
  int i,indexKing;

  int sr,kr;

  char sc,kc;

  indexKing=0;

  /*Loop for finds out the place of the given king*/
  for(i=0;i<64;i++)
  {
    if(board[i]==king)
    {
      indexKing=i;
    }
  }

  /*Loop for find the piece which check the king*/
  for(i=0;i<64;i++)
  {
    /*Turning the index to the col and row*/
    kr=8-(int)(indexKing/8.0);
    kc='a'+(indexKing-(int)(indexKing/8.0)*8);

    sr=8-(int)(i/8.0);
    sc='a'+(i-(int)(i/8.0)*8);

    /*if king is black,the enemy pieces only whites*/
    if(king=='k')
    {
      if(board[i]>='A' && board[i]<='Z')
      {
        if(isPieceMovable(&sc,&sr,&kc,&kr))
        {
          return 1;
        }
      }
    }
    /*if king is white,the enemy pieces only blacks*/
    else if(king=='K')
    {
      if(board[i]>='a' && board[i]<='z')
      {
        
        if(isPieceMovable(&sc,&sr,&kc,&kr))
        {
          return 1;
        }
      }
    }

  }

  return 0;
}


/*This func check the rook is movable according to rules*/
int isRookMovable(char *sc,int *sr,char *tc, int *tr)
{
    int piece,i,check,targetpos;


    /*Find the index of source and target cells for array using*/
    piece=(int)((*sc-'a')+(8-*sr)*8);
    targetpos=(int)((*tc-'a')+(8-*tr)*8);

    check=isValidCell2(board,sc,sr,tc,tr);
    
    if(check)
    {
    /*According to the game rules, rook can move only the column changed or  */
            /*          only the row was changed not both of it         */
        if(sr!=tr && sc==tc)
        {
            /*Loop for checking every cell one by one until the target cell*/
            for(i=1;i<=abs(tr-sr);i++)
            {
                if(abs(tr-sr)==i)
                {
                   /*if the enemy piece in the target cell,rook can move there*/
                    if(board[piece]>='a' && board[piece]<='z')
                    {
                        if(board[targetpos]>='A' && board[targetpos]<='Z')
                        {
                            return 1;
                        }
                    }
                    else if(board[piece]>='A' && board[piece]<='Z')
                    {
                        if(board[targetpos]>='a' && board[targetpos]<='z')
                        {
                            return 1;
                        }
                    }
                }
            /*if the target row greater than source row we re going to the up*/
            /*       So our index should decrease                  */
                if(sr<tr)
                {
                    if(board[piece-8*i]!=' ')
                    {
                        return 0;
                    }
                }
                else if(tr<sr)
                {
                    if(board[piece+8*i]!=' ')
                    {
                        return 0;
                    }
                }
            }
        }
        else if(sc!=tc && sr==tr)
        {
            for(i=1;i<=abs((int)(tc-sc));i++)
            {
                if(abs(tc-sc)==i)
                {

                    if(board[piece]>='a' && board[piece]<='z')
                    {
                        if(board[targetpos]>='A' && board[targetpos]<='Z')
                        {
                            return 1;
                        }
                    }
                    else if(board[piece]>='A' && board[piece]<='Z')
                    {
                        if(board[targetpos]>='a' && board[targetpos]<='z')
                        {
                            return 1;
                        }
                    }
                }

                if(sc<tc)
                {
                    if(board[piece+i]!=' ')
                    {
                        return 0;
                    }
                }
                else if(tc<sc)
                {
                    if(board[piece-i]!=' ')
                    {
                        return 0;
                    }
                }
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

/*This func check the knight is movable according to rules*/
int isKnightMovable(char *sc,int *sr,char *tc, int *tr)
{
    int check,targetpos,sourcepos;
    /*Check the validness*/
    check=isValidCell2(board,sc,sr,tc,tr);

    /*Find the index of source and target cells for array using*/
    sourcepos=(int)((*sc-'a')+(8-*sr)*8);
    targetpos=(int)((*tc-'a')+(8-*tr)*8);

    /*Knight can move everywhere but only like a 'L' letter.
                                            So there re four possibilities*/
    if(check)
    {
        if(abs(targetpos-sourcepos)==15||abs(targetpos-sourcepos)==6
            ||abs(targetpos-sourcepos)==10||abs(targetpos-sourcepos)==17)
        {
            return 1;
        }
    }
    
    return 0;
}

/*This func check the pawn is movable according to rules*/
int isPawnMovable(char *sc,int *sr,char *tc, int *tr)
{
    int check,sourcepos,targetpos;

    check=isValidCell2(board,sc,sr,tc,tr);
    /*Find the index of source and target cells for array using*/
    sourcepos=(int)((*sc-'a')+(8-*sr)*8);
    targetpos=(int)((*tc-'a')+(8-*tr)*8);

    /*The pawn move only 1 cell in our rules,so there re 3 possibilities*/
    if(check)
    {
        if(sourcepos-targetpos==8 && board[targetpos]==' ' && 
                                                        board[sourcepos]=='P')
        {
            return 1;
        }
        else if(targetpos-sourcepos==8 && board[targetpos]==' ' &&
                                                         board[sourcepos]=='p')
        {
            return 1;
        }
        else if(sourcepos-targetpos==7 || sourcepos-targetpos==9)
        {
                if(board[sourcepos]=='P')
                {
                    if(board[targetpos]>='a'&&board[targetpos]<='z')
                    {
                        return 1;
                    }
                }
        }
        else if(targetpos-sourcepos==7 || targetpos-sourcepos==9)
        {
                if(board[sourcepos]=='p')
                {
                    if(board[targetpos]>='A'&&board[targetpos]<='Z')
                    {
                        return 1;
                    }
                }
        }
    }
    return 0;
}

/*This func check the bishop is movable according to rules*/
int isBishopMovable(char *sc,int *sr,char *tc, int *tr)
{
    int check,piece,i,targetpos,a,b;

    char curCol;

    int curRow;

    piece=(int)((*sc-'a')+(8-*sr)*8);
    targetpos=(int)((*tc-'a')+(8-*tr)*8);
    /*Check the validness*/

    check=isValidCell2(board,sc,sr,tc,tr);

    a=abs(sc-tc);
    b=abs(sr-tr);

    if(check && a==b)
    {   /*Loop for checking every cell one by one until the target cell*/
        for(i=1;i<=abs(tr-sr);i++)
        {
            if(abs(tr-sr)==i)
            {
                /*if the enemy piece in the target cell,rook can move there*/
                if(board[piece]>='a' && board[piece]<='z')
                {
                    if(board[targetpos]>='A' && board[targetpos]<='Z')
                    {
                        return 1;
                    }
                }
                else if(board[piece]>='A' && board[piece]<='Z')
                {
                    if(board[targetpos]>='a' && board[targetpos]<='z')
                    {
                        return 1;
                    }
                }
            }
    /*For checking the bishop movabeleness we can think our board 
                        like four equal pieces,so we have four possibilities*/  
            
            


            if(sr<tr && tc>sc)
            {
                curCol='a'+(piece-7*i)-(int)((piece-7*i)/8.0)*8;
                curRow=8-(int)((piece-7*i)/8.0);
                
                if((curCol=='h' || curCol=='a' || curRow==8 || curRow==1) && 
                                                                abs(tr-sr)!=i)
                {
                    return 0;
                }
                else if(board[piece-7*i]!=' ')
                {

                    return 0;
                }
            }
            else if(tr<sr && tc>sc)
            {
                curCol='a'+(piece+9*i)-(int)((piece+9*i)/8.0)*8;
                curRow=8-(int)((piece+9*i)/8.0);
                
                if((curCol=='h' || curCol=='a' || curRow==8 || curRow==1) && 
                                                                abs(tr-sr)!=i)
                {
                    return 0;
                }
                else if(board[piece+9*i]!=' ')
                {
                    return 0;
                }
            }
            else if(sr<tr && tc<sc)
            {
                curCol='a'+(piece-9*i)-(int)((piece-9*i)/8.0)*8;
                curRow=8-(int)((piece-9*i)/8.0);
                
                if((curCol=='h' || curCol=='a' || curRow==8 || curRow==1) && 
                                                                abs(tr-sr)!=i)
                {
                    return 0;
                }
                else if(board[piece-9*i]!=' ')
                {
                    return 0;
                }
            }
            else if(tr<sr && tc<sc)
            {
                curCol='a'+(piece+7*i)-(int)((piece+7*i)/8.0)*8;
                curRow=8-(int)((piece+7*i)/8.0);
                
                if((curCol=='h' || curCol=='a' || curRow==8 || curRow==1) &&
                                                                 abs(tr-sr)!=i)
                {
                    return 0;
                }
                else if(board[piece+7*i]!=' ')
                {
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

/*This func check the queen is movable according to rules*/
int isQueenMovable(char *sc,int *sr,char *tc, int *tr)
{
    int checkTarget;

    checkTarget=isValidCell2(board,sc,sr,tc,tr);
 /*The queen movable func is much more simple because the queen move like
                                                            bishop and rook*/
    if(checkTarget)
    {
    /*if rook or bishop func return 1; it will return 1 too. Otherwise 0*/
        if(isRookMovable(sc,sr,tc,tr) || 
            isBishopMovable(sc,sr,tc,tr))
        {
            return 1;
        }
    }

    return 0;
}

  /*This func check the king is movable according to rules*/
int isKingMovable(char *sc,int *sr,char *tc, int *tr)
{
    int checkTarget,sourcepiece,targetpiece;

    sourcepiece=(int)((*sc-'a')+(8-*sr)*8);
    targetpiece=(int)((*tc-'a')+(8-*tr)*8);

    checkTarget=isValidCell2(board,sc,sr,tc,tr);

    if(checkTarget)
    {
        /*The king can move only one piece around so we have 4 possibilities*/
        
        if(abs(sourcepiece-targetpiece)==1 || abs(sourcepiece-targetpiece)==7||
           abs(sourcepiece-targetpiece)==8 || abs(sourcepiece-targetpiece)==9)
        {
          /*if rook or bishop func return 1; it will return 1 too. Otherwise 0*/
            if(isRookMovable(sc,sr,tc,tr) || 
                isBishopMovable(sc,sr,tc,tr))
            {
                return 1;
            }

        }
        
    }
    

    return 0;
}




















