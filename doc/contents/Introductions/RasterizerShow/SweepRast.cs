using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace RasterizerShow
{
	public partial class SweepRast : Form
	{
		public SweepRast()
		{
			InitializeComponent();
		}

		private void SweepRast_MouseClick(object sender, MouseEventArgs e)
		{
			ras = new Raster();
			ras.Width = rWidth;
			ras.Height = rHeight;
			ras.DrawTiles();

			if (step >= 1)
			{
				ras.ActiveCell(9, 2, false);
			}

			if (step >= 2)
			{
				ras.ActiveCell(9, 2, true);
			}

			Point[] tileSeq
				= new Point[]{
					new Point(2,0),
					new Point(3,0),
					new Point(1,0),
					new Point(0,0),
					new Point(1,1),
					new Point(0,1),
					new Point(2,1),
					new Point(3,1),
					new Point(2,2),
					new Point(1,2),
					new Point(0,2),
					new Point(3,2),
					new Point(2,3),
					new Point(1,3),
					new Point(0,3),
					new Point(3,3),
				};

			if (step >= 3)
			{
				int maxId = Math.Min( tileSeq.Length, step-3 );
				for (int i = 0; i < maxId; ++i)
				{
					ActiveCellInTileAndTriangle(tileSeq[i].X, tileSeq[i].Y, true);
					ras.DrawTile(tileSeq[i].X, tileSeq[i].Y, true, Raster.Mask.Pending);
				}
			}

			Bitmap bmp = ras.Bitmap.Clone() as Bitmap;
			Utility.DrawTriangle(bmp, pt0, pt1, pt2, ras.CellSize);

			++step;
			this.BackgroundImage = bmp;
		}

		void ActiveCellInTileAndTriangle( int tileX, int tileY, bool affectsTile )
		{
			for (int i = tileX * ras.TileSize; i < (tileX + 1)*ras.TileSize; ++i) 
			{
				for (int j = tileY * ras.TileSize; j < (tileY + 1) * ras.TileSize; ++j)
				{
					if (Utility.PositiveCellOfLine(i, j, pt0, pt1)
						&& Utility.PositiveCellOfLine(i, j, pt1, pt2)
						&& Utility.PositiveCellOfLine(i, j, pt2, pt0)
						)
					{
						ras.ActiveCell(i, j, affectsTile);
					}
				}
			}
		}

		private PointF pt0 = new PointF(9.7F, 1.8F);
		private PointF pt1 = new PointF(1.3F, 7.6F);
		private PointF pt2 = new PointF(13.1F, 14.8F);

		private Raster ras;
		private int rWidth = 16;
		private int rHeight = 16;

		private int step = 0;
	}
}
