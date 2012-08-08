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
	public partial class Rast : Form
	{
		public Rast()
		{
			InitializeComponent();
		}

		private void button1_Click(object sender, EventArgs e)
		{
			SLRast wnd = new SLRast();
			wnd.ShowDialog();
		}

		private void button2_Click(object sender, EventArgs e)
		{
			SweepRast wnd = new SweepRast();
			wnd.ShowDialog();
		}

		private void button3_Click(object sender, EventArgs e)
		{
			LRBRast wnd = new LRBRast();
			wnd.ShowDialog();
		}
	}
}
