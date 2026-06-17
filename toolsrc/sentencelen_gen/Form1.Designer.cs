namespace sentencelen_gen
{
    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            importSentences = new Button();
            saveSentenceLen = new Button();
            sentenceList = new DataGridView();
            name = new DataGridViewTextBoxColumn();
            len = new DataGridViewTextBoxColumn();
            ((System.ComponentModel.ISupportInitialize)sentenceList).BeginInit();
            SuspendLayout();
            // 
            // importSentences
            // 
            importSentences.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            importSentences.Location = new Point(12, 12);
            importSentences.Name = "importSentences";
            importSentences.Size = new Size(415, 42);
            importSentences.TabIndex = 0;
            importSentences.Text = "Import sentences.txt";
            importSentences.UseVisualStyleBackColor = true;
            importSentences.Click += importSentences_Click;
            // 
            // saveSentenceLen
            // 
            saveSentenceLen.Anchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            saveSentenceLen.Location = new Point(12, 428);
            saveSentenceLen.Name = "saveSentenceLen";
            saveSentenceLen.Size = new Size(415, 31);
            saveSentenceLen.TabIndex = 1;
            saveSentenceLen.Text = "Generate sentencelen.txt";
            saveSentenceLen.UseVisualStyleBackColor = true;
            saveSentenceLen.Click += saveSentenceLen_Click;
            // 
            // sentenceList
            // 
            sentenceList.AllowUserToAddRows = false;
            sentenceList.AllowUserToDeleteRows = false;
            sentenceList.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            sentenceList.AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode.Fill;
            sentenceList.ColumnHeadersHeightSizeMode = DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            sentenceList.Columns.AddRange(new DataGridViewColumn[] { name, len });
            sentenceList.Location = new Point(12, 60);
            sentenceList.MultiSelect = false;
            sentenceList.Name = "sentenceList";
            sentenceList.ReadOnly = true;
            sentenceList.RightToLeft = RightToLeft.No;
            sentenceList.ScrollBars = ScrollBars.Vertical;
            sentenceList.Size = new Size(415, 362);
            sentenceList.TabIndex = 2;
            // 
            // name
            // 
            name.HeaderText = "Name";
            name.Name = "name";
            name.ReadOnly = true;
            // 
            // len
            // 
            len.HeaderText = "Length";
            len.Name = "len";
            len.ReadOnly = true;
            // 
            // Form1
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(439, 471);
            Controls.Add(sentenceList);
            Controls.Add(saveSentenceLen);
            Controls.Add(importSentences);
            Name = "Form1";
            Text = "SentenceLen Gen";
            ((System.ComponentModel.ISupportInitialize)sentenceList).EndInit();
            ResumeLayout(false);
        }

        #endregion

        private Button importSentences;
        private Button saveSentenceLen;
        private DataGridView sentenceList;
        private DataGridViewTextBoxColumn name;
        private DataGridViewTextBoxColumn len;
    }
}
