using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Drawing.Drawing2D;

namespace RasterizerShow
{
	public class Raster
	{
		public const int TopLeft = 1 << 3;
		public const int BottomLeft = 1 << 2;
		public const int TopRight = 1 << 1;
		public const int BottomRight = 1 << 0;

		public enum Mask
		{
			None,
			Accepted,
			Rejected,
			Pending
		}

		public Bitmap Bitmap
		{
			get { return bmp; }
		}

		public Color GridColor
		{
			get { return gColor; }
			set { gColor = value; }
		}

		public Color LineColor
		{
			get { return lColor; }
			set { lColor = value; }
		}

		public int CellSize
		{
			get { return cellSize; }
			set { cellSize = value; Update(false); }
		}

		public int TileSize
		{
			get { return tileSize; }
		}

		public int TileWidth
		{
			get { return (width + tileSize - 1) / tileSize; }
		}

		public int TileHeight
		{
			get { return (height + tileSize - 1) / tileSize; }
		}

		public int Width
		{
			get { return width; }
			set { width = value; Update(true); }
		}

		public int Height
		{
			get { return height; }
			set { height = value; Update(true); }
		}

		public int CenterSize
		{
			get { return centerSize; }
			set { centerSize = value; Update(false); }
		}

		public void DrawTiles()
		{
			for( int i = 0; i < (width + tileSize - 1) / tileSize; ++i ){
				for (int j = 0; j < (height + tileSize - 1) / tileSize; ++j )
				{
					DrawTile(i, j, false);
				}
			}
		}

		public void ActiveCell(int x, int y, bool affectsTile)
		{
			actives[y, x] = true;
			DrawCell(x, y);
			if (affectsTile)
			{
				DrawTile(x / tileSize, y / tileSize, true);
			}
		}

		public void InactiveCell(int x, int y, bool affectsTile)
		{
			actives[y, x] = false;
			DrawCell(x, y);
			if (affectsTile)
			{
				DrawTile(x / tileSize, y / tileSize, false);
			}
		}

		private void Update( bool updateActive )
		{
			if( width == 0 || height == 0 || cellSize == 0 )
			{
				return;
			}

			int destWidth = width * (cellSize + 2);
			int destHeight = height * (cellSize + 2);
			if ( bmp == null || bmp.Width < destWidth || bmp.Height < destHeight )
			{
				bmp = new Bitmap(destWidth, destHeight);
			}

			if (updateActive)
			{
				actives = new bool[height, width];
			}
			for (int i = 0; i < height; ++i)
			{
				for (int j = 0; j < width; ++j)
				{
					if (updateActive)
					{
						actives[i, j] = false;
					}
					DrawCell(j, i);
				}
			}
		}

		private void DrawCell(int x, int y)
		{
			Graphics g = Graphics.FromImage(bmp);
			Color cColor = actives[y, x] ? aColor : gColor;
			g.FillRectangle(new SolidBrush(cColor), x * ( cellSize + 2 )+1, y * (cellSize + 2)+1, cellSize, cellSize);
			g.DrawRectangle(new Pen(lColor), x * (cellSize + 2), y * (cellSize + 2), cellSize + 1, cellSize + 1);
			g.FillEllipse(new SolidBrush(centerColor),
				x * (cellSize + 2) + cellSize / 2 - centerSize / 2,
				y * (cellSize + 2) + cellSize / 2 - centerSize / 2,
				centerSize,
				centerSize
				);
		}

		public void DrawTile(int x, int y, bool isActive, Mask mask = Mask.None, int trivialCode = 0 )
		{
			Graphics g = Graphics.FromImage(bmp);
			Color c = isActive ? Color.Red : Color.White;
			g.DrawRectangle(
			    new Pen(c),
			    x * tileSize * (cellSize + 2),
			    y * tileSize * (cellSize + 2),
			    (cellSize + 2) * tileSize - 1,
			    (cellSize + 2) * tileSize - 1
			    );

			if( mask == Mask.Accepted ){
				HatchBrush b = new HatchBrush(HatchStyle.BackwardDiagonal, Color.Blue, Color.Transparent);

				g.FillRectangle(
					b,
					x * tileSize * (cellSize + 2),
					y * tileSize * (cellSize + 2),
					(cellSize + 2) * tileSize - 1,
					(cellSize + 2) * tileSize - 1
					);
			}

			if (mask == Mask.Rejected)
			{
				Brush b = new HatchBrush(HatchStyle.DiagonalCross, Color.Blue, Color.Transparent);
				g.FillRectangle(
					b,
					x * tileSize * (cellSize + 2),
					y * tileSize * (cellSize + 2),
					(cellSize + 2) * tileSize - 1,
					(cellSize + 2) * tileSize - 1
					);
			}

			if (mask == Mask.Pending)
			{
				Brush b = new HatchBrush(HatchStyle.DottedGrid, Color.Blue, Color.Transparent);
				g.FillRectangle(
					b,
					x * tileSize * (cellSize + 2),
					y * tileSize * (cellSize + 2),
					(cellSize + 2) * tileSize - 1,
					(cellSize + 2) * tileSize - 1
					);
			}
			//Draw trivial points

			// TOP LEFT
			if ( (trivialCode & TopLeft) > 0)
			{
				float xcoord = x * tileSize * (cellSize + 2);
				float ycoord = y * tileSize * (cellSize + 2);

				g.FillEllipse(new SolidBrush(trivialColor),
					xcoord - centerSize / 2,
					ycoord - centerSize / 2,
					centerSize,
					centerSize
					);
			}

			if ((trivialCode & TopRight) > 0)
			{
				float xcoord = (x+1) * tileSize * (cellSize + 2);
				float ycoord = y * tileSize * (cellSize + 2);

				g.FillEllipse(new SolidBrush(trivialColor),
					xcoord - centerSize / 2,
					ycoord - centerSize / 2,
					centerSize,
					centerSize
					);
			}

			if ((trivialCode & BottomLeft) > 0)
			{
				float xcoord = x * tileSize * (cellSize + 2);
				float ycoord = (y+1) * tileSize * (cellSize + 2);

				g.FillEllipse(new SolidBrush(trivialColor),
					xcoord - centerSize / 2,
					ycoord - centerSize / 2,
					centerSize,
					centerSize
					);
			}

			if ((trivialCode & BottomRight) > 0)
			{
				float xcoord = (x+1) * tileSize * (cellSize + 2);
				float ycoord = (y+1) * tileSize * (cellSize + 2);

				g.FillEllipse(new SolidBrush(trivialColor),
					xcoord - centerSize / 2,
					ycoord - centerSize / 2,
					centerSize,
					centerSize
					);
			}
		}

		private Color gColor = Color.DarkGray;
		private Color aColor = Color.Blue;
		private Color lColor = Color.Black;
		private Color centerColor = Color.White;
		private Color trivialColor = Color.Yellow;

		private Bitmap bmp;

		private int tileSize = 4;
		private int cellSize = 32;
		private int centerSize = 7;
		private int width;
		private int height;
		private bool[,] actives;
	}
}
