using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;

namespace RasterizerShow
{
	public class Utility
	{
		public static bool PositiveCellOfLine(int x, int y, PointF pt0, PointF pt1)
		{
			float diffY = pt1.Y - pt0.Y;
			float diffX = pt1.X - pt0.X;

			return ((float)y + 0.5F) * diffX - pt0.Y * diffX - ((float)x + 0.5F - pt0.X) * diffY < 0;
		}

		public static bool PositivePointOfLine(int x, int y, PointF pt0, PointF pt1)
		{
			float diffY = pt1.Y - pt0.Y;
			float diffX = pt1.X - pt0.X;

			return ((float)y) * diffX - pt0.Y * diffX - ((float)x - pt0.X) * diffY < 0;
		}

		public static bool TrivialPositive(int x, int y, int width, int height, PointF pt0, PointF pt1)
		{
			int radiusW = width / 2;
			int radiusH = height / 2;

			int centerX = x + radiusW;
			int centerY = y + radiusH;

			float diffX = pt1.X - pt0.X;
			float diffY = pt1.Y - pt0.Y;

			return PositivePointOfLine(centerX + Math.Sign(-diffY) * radiusW, centerY + Math.Sign(diffX)* radiusH, pt0, pt1);
		}

		public static bool TrivialNegative(int x, int y, int width, int height, PointF pt0, PointF pt1)
		{

			int radiusW = width / 2;
			int radiusH = height / 2;

			int centerX = x + radiusW;
			int centerY = y + radiusH;

			float diffX = pt1.X - pt0.X;
			float diffY = pt1.Y - pt0.Y;

			return !PositivePointOfLine(centerX + Math.Sign(diffY) * radiusW, centerY + Math.Sign(-diffX) * radiusH, pt0, pt1);
		}

		public static void DrawTriangle(Bitmap bmp, PointF pt0, PointF pt1, PointF pt2, int cellSize)
		{
			PointF spt0 = ToScreenSpace(pt0, cellSize);
			PointF spt1 = ToScreenSpace(pt1, cellSize);
			PointF spt2 = ToScreenSpace(pt2, cellSize);

			Pen p = Pens.Yellow.Clone() as Pen;
			p.Width = 2.0F;

			Graphics g = Graphics.FromImage(bmp);
			g.DrawPolygon(p, new PointF[] { spt0, spt1, spt2 });
		}

		public static void DrawLine(Bitmap bmp, PointF pt0, PointF pt1, int cellSize)
		{
			SizeF diff = new SizeF(pt1.X - pt0.X, pt1.Y - pt0.Y);
			diff = Resize(diff, (float)(Length(bmp.Size) / cellSize));
			Graphics g = Graphics.FromImage(bmp);

			Pen p = Pens.Yellow.Clone() as Pen;
			p.DashStyle = System.Drawing.Drawing2D.DashStyle.DashDot;
			p.DashPattern = new float[]{7, 7};
			p.Width = 2.0F;

			PointF sp0 = ToScreenSpace(PointF.Add(pt0, diff), cellSize);
			PointF sp1 = ToScreenSpace(PointF.Subtract(pt0, diff), cellSize);
			g.DrawLine(p, sp0, sp1 );
		}

		public static SizeF Resize(SizeF pt, float len)
		{
			float scale = len / Length(pt);
			return new SizeF(pt.Width * scale, pt.Height * scale);
		}

		public static float Length(SizeF pt)
		{
			return (float)Math.Sqrt(pt.Width * pt.Width + pt.Height * pt.Height);
		}

		public static PointF ToScreenSpace(PointF pt, int cellSize)
		{
			return new PointF(pt.X * (cellSize + 2), pt.Y * (cellSize + 2));
		}
	}
}
