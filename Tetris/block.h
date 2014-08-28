/*******下落方块的结构体struct Block*******/
struct BLOCK
{
	RECT rectBlock;
	int Map[4][4];			//下降的方块放置在widthXheight的rectBlock种，Map纪录每个小方块所在的位置
	int Colour;				//当前下降方块的颜色，共五种，分别代表五大类形状的颜色，长条，L型，Z型，凸型，田型
	int Classifier;			//总共有7种形式的方块，长条(1种)，L(2种)型，Z型(2种)，凸型(1种)和田字型(1种)
	int Style;					//当前的样式，
};//指向当前的方块和指向下一个方块

/**********************************END*/
