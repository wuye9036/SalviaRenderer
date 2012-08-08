using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Drawing2D;

namespace RasterizerShow
{
	public partial class SLRast : Form
	{
		public SLRast()
		{
			InitializeComponent();
		}

		private void DrawHorizontalSplitter( Bitmap bmp, PointF pt0, PointF pt1, PointF pt2 )
		{
			pt0.X *= (ras.CellSize + 2);
			pt0.Y *= (ras.CellSize + 2);

			pt1.X *= (ras.CellSize + 2);
			pt1.Y *= (ras.CellSize + 2);

			pt2.X *= (ras.CellSize + 2);
			pt2.Y *= (ras.CellSize + 2);

			Graphics g = Graphics.FromImage(bmp);

			float diffY = pt2.Y - pt0.Y;
			float diffX = pt2.X - pt0.X;

			float offset = ras.CellSize / 1.5F;

			Pen p = new Pen(Color.Red, 2.0F);
			p.DashStyle = DashStyle.Dash;
			p.DashCap = DashCap.Round;
			p.DashPattern = new float[] { 8.0F, 4.0F };
			g.DrawLine(
				p,
				pt1.X - offset, pt1.Y, (pt1.Y - pt0.Y) * (diffX / diffY) + pt0.X + offset, pt1.Y
				);
		}

		private void Form1_MouseClick(object sender, MouseEventArgs e)
		{
			ras = new Raster();
			ras.Width = rWidth;
			ras.Height = rHeight;

			int lineCount = 0;

			for (int j = 0; j < rHeight; ++j)
			
			{
				bool lineActive = false;

				for (int i = 0; i < rWidth; ++i)
				{
					if ( Utility.PositiveCellOfLine(i, j, pt0, pt1)
						&& Utility.PositiveCellOfLine(i, j, pt1, pt2) 
						&& Utility.PositiveCellOfLine(i, j, pt2, pt0) 
						)
					{
						lineActive = true;
						ras.ActiveCell(i, j, false);
					}
				}

				if (lineActive) { ++lineCount; }
				if (lineCount >= step - 1)
				{
					break;
				}
			}
			
			Bitmap bmp = new Bitmap(ras.Bitmap);

			Utility.DrawTriangle( bmp, pt0, pt1, pt2, ras.CellSize);
			if (step >= 1)
			{
				DrawHorizontalSplitter(bmp, pt0, pt1, pt2);
			}

			++step;

			this.BackgroundImage = bmp;
			this.BackgroundImageLayout = ImageLayout.Center;
		}

		private PointF pt0 = new PointF(9.7F, 1.8F);
		private PointF pt1 = new PointF(1.3F, 7.6F);
		private PointF pt2 = new PointF(13.1F, 14.8F);

		private Raster ras;
		private int rWidth = 16;
		private int rHeight = 16;

		private int step = 0;

		private void button1_Click(object sender, EventArgs e)
		{
			if (this.BackgroundImage != null)
			{
				Clipboard.SetImage(this.BackgroundImage);
			}
		}
	}
}
