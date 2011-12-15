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
	public partial class LRBRast : Form
	{
		public LRBRast()
		{
			InitializeComponent();

			linePts[0] = new PointF(5.7f, 3.2f);
			linePts[1] = new PointF(3.4f, 4.6f);
			linePts[2] = new PointF(10.1f, 9.2f);

		}

		public void DrawSteps(int iStep)
		{
			ras.CellSize = 32;
			ras.Width = ras.Height = 16;

			Bitmap bmp = ras.Bitmap;

			switch (iStep)
			{
				case 1:
					break;
				case 2:
					Utility.DrawTriangle(bmp, linePts[0], linePts[1], linePts[2], ras.CellSize);
					break;
				case 3:
					Utility.DrawLine(bmp, linePts[0], linePts[1], ras.CellSize);
					Utility.DrawTriangle(bmp, linePts[0], linePts[1], linePts[2], ras.CellSize);
					break;
				case 4:
					ras.DrawTiles();
					bmp = ras.Bitmap;
					Utility.DrawLine(bmp, linePts[0], linePts[1], ras.CellSize);
					Utility.DrawTriangle(bmp, linePts[0], linePts[1], linePts[2], ras.CellSize);
					break;
				case 5:
					ras.DrawTiles();
					ras.DrawTile(0, 0, true, Raster.Mask.None, 0xFF);
					bmp = ras.Bitmap;
					Utility.DrawLine(bmp, linePts[0], linePts[1], ras.CellSize);
					Utility.DrawTriangle(bmp, linePts[0], linePts[1], linePts[2], ras.CellSize);
					break;
				case 6:
					ras.DrawTiles();
					ras.DrawTile(0, 0, true, Raster.Mask.Rejected, Raster.BottomRight);
					bmp = ras.Bitmap;
					Utility.DrawLine(bmp, linePts[0], linePts[1], ras.CellSize);
					Utility.DrawTriangle(bmp, linePts[0], linePts[1], linePts[2], ras.CellSize);
					break;
				case 7:
					ras.DrawTiles();
					ras.DrawTile(2, 2, true, Raster.Mask.Accepted, Raster.TopLeft);
					bmp = ras.Bitmap;
					Utility.DrawLine(bmp, linePts[0], linePts[1], ras.CellSize);
					Utility.DrawTriangle(bmp, linePts[0], linePts[1], linePts[2], ras.CellSize);
					break;
				case 8:
					ras.DrawTiles();
					ras.DrawTile(1, 1, true, Raster.Mask.Pending, Raster.TopLeft | Raster.BottomRight);
					bmp = ras.Bitmap;
					Utility.DrawLine(bmp, linePts[0], linePts[1], ras.CellSize);
					Utility.DrawTriangle(bmp, linePts[0], linePts[1], linePts[2], ras.CellSize);
					break;
				case 9:
					for (int i = 0; i < ras.TileWidth; ++i)
					{
						for (int j = 0; j < ras.TileHeight; ++j)
						{
							if (Utility.TrivialNegative(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[0], linePts[1]))
							{
								ras.DrawTile(i, j, true, Raster.Mask.Rejected, Raster.BottomRight);
							}
							else if (Utility.TrivialPositive(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[0], linePts[1]))
							{
							    ras.DrawTile(i, j, true, Raster.Mask.Accepted, Raster.TopLeft);
							}
							else
							{
								ras.DrawTile(i, j, true, Raster.Mask.Pending, Raster.TopLeft);
							}

						}
					}
					
					bmp = ras.Bitmap;
					Utility.DrawTriangle(bmp, linePts[0], linePts[1], linePts[2], ras.CellSize);
					Utility.DrawLine(bmp, linePts[0], linePts[1], ras.CellSize);
					break;
				case 10:
					for (int i = 0; i < ras.TileWidth; ++i)
					{
						for (int j = 0; j < ras.TileHeight; ++j)
						{
							if (
								Utility.TrivialPositive(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[0], linePts[1]) &&
								Utility.TrivialPositive(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[1], linePts[2]) &&
								Utility.TrivialPositive(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[2], linePts[0])
								)
							{
								ras.DrawTile(i, j, true, Raster.Mask.Accepted);
							}
							else if (
								Utility.TrivialNegative(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[0], linePts[1]) ||
								Utility.TrivialNegative(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[1], linePts[2]) ||
								Utility.TrivialNegative(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[2], linePts[0])
								)
							{
								ras.DrawTile(i, j, true, Raster.Mask.Rejected);
							}
							else
							{
								ras.DrawTile(i, j, true, Raster.Mask.Pending);
							}
						}
					}
					Utility.DrawTriangle(bmp, linePts[0], linePts[1], linePts[2], ras.CellSize);
					break;
				case 11:
					for (int i = 0; i < ras.TileWidth; ++i)
					{
						for (int j = 0; j < ras.TileHeight; ++j)
						{
							if (
								Utility.TrivialPositive(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[0], linePts[1]) &&
								Utility.TrivialPositive(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[1], linePts[2]) &&
								Utility.TrivialPositive(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[2], linePts[0])
								)
							{
								ras.DrawTile(i, j, true, Raster.Mask.Accepted);
							}
							else if (
								Utility.TrivialNegative(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[0], linePts[1]) ||
								Utility.TrivialNegative(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[1], linePts[2]) ||
								Utility.TrivialNegative(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[2], linePts[0])
								)
							{
								ras.DrawTile(i, j, true, Raster.Mask.Rejected);
							}
							else
							{
								ras.DrawTile(i, j, true, Raster.Mask.Pending);
							}
						}
					}
					Utility.DrawTriangle(bmp, linePts[0], linePts[1], linePts[2], ras.CellSize);
					Utility.DrawLine(bmp, linePts[0], linePts[1], ras.CellSize);
					Utility.DrawLine(bmp, linePts[1], linePts[2], ras.CellSize);
					Utility.DrawLine(bmp, linePts[2], linePts[0], ras.CellSize);
					break;
				case 12:
					linePts[0] = new PointF(5.0f, 1.0f);
					linePts[1] = new PointF(0.8f, 13.0f);
					linePts[2] = new PointF(14.4f, 10.0f);

					for (int i = 0; i < ras.TileWidth; ++i)
					{
						for (int j = 0; j < ras.TileHeight; ++j)
						{
							if (
								Utility.TrivialPositive(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[0], linePts[1]) &&
								Utility.TrivialPositive(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[1], linePts[2]) &&
								Utility.TrivialPositive(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[2], linePts[0])
								)
							{
								ras.DrawTile(i, j, true, Raster.Mask.Accepted);
							}
							else if (
								Utility.TrivialNegative(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[0], linePts[1]) ||
								Utility.TrivialNegative(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[1], linePts[2]) ||
								Utility.TrivialNegative(i * ras.TileSize, j * ras.TileSize, ras.TileSize, ras.TileSize, linePts[2], linePts[0])
								)
							{
								ras.DrawTile(i, j, true, Raster.Mask.Rejected);
							}
							else
							{
								ras.DrawTile(i, j, true, Raster.Mask.Pending);
							}
						}
					}
					Utility.DrawTriangle(bmp, linePts[0], linePts[1], linePts[2], ras.CellSize);
					Utility.DrawLine(bmp, linePts[0], linePts[1], ras.CellSize);
					Utility.DrawLine(bmp, linePts[1], linePts[2], ras.CellSize);
					Utility.DrawLine(bmp, linePts[2], linePts[0], ras.CellSize);
					break;
			}

			Text = string.Format("Larrabee Rasterizer Demo: {0}", iStep);

			BackgroundImageLayout = ImageLayout.Center;
			BackgroundImage = bmp;
			Refresh();

			return;
		}

		private void LRBRast_MouseClick(object sender, MouseEventArgs e)
		{
			if (e.Button == System.Windows.Forms.MouseButtons.Right)
			{
				DrawSteps(--step);
			}
			else
			{
				DrawSteps(++step);
			}

		}

		private int step = 0;
		private Raster ras = new Raster();
		private PointF[] linePts = new PointF[3];
	}
}
