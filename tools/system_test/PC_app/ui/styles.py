APP_STYLE = """
QMainWindow
{
    background-color: #F4F6F8;
    color: #111827;
    font-family: Segoe UI, Arial, sans-serif;
}

QWidget
{
    font-size: 10pt;
}

QLabel#title
{
    font-size: 22pt;
    font-weight: 700;
    color: #111827;
}

QLabel#subtitle
{
    color: #64748B;
}

QLabel#sectionTitle
{
    font-size: 12pt;
    font-weight: 700;
    color: #111827;
}

QLabel#muted
{
    color: #64748B;
}

QLabel#metricValue
{
    font-size: 24pt;
    font-weight: 700;
    color: #111827;
}

QLabel#metricUnit
{
    color: #64748B;
}

QLabel#pill
{
    border-radius: 8px;
    padding: 5px 10px;
    font-weight: 600;
}

QLabel#pill[status="ok"]
{
    background: #DCFCE7;
    color: #166534;
}

QLabel#pill[status="bad"]
{
    background: #FEE2E2;
    color: #991B1B;
}

QLabel#pill[status="idle"]
{
    background: #E2E8F0;
    color: #334155;
}

QFrame#card
{
    background: #FFFFFF;
    border-radius: 8px;
    border: 1px solid #E2E8F0;
}

QFrame#topBar
{
    background: #FFFFFF;
    border-radius: 8px;
    border: 1px solid #E2E8F0;
}

QGroupBox
{
    border: 1px solid #E2E8F0;
    border-radius: 6px;
    margin-top: 8px;
    padding: 8px 6px 6px 6px;
    color: #334155;
    font-weight: 600;
}

QGroupBox::title
{
    subcontrol-origin: margin;
    left: 10px;
    padding: 0 4px;
}

QPushButton
{
    background: #FFFFFF;
    border: 1px solid #CBD5E1;
    border-radius: 6px;
    padding: 6px 10px;
    min-height: 24px;
    font-size: 10pt;
    font-weight: 600;
    color: #0F172A;
}

QPushButton:hover
{
    border: 1px solid #2563EB;
    background: #F8FAFC;
}

QPushButton:pressed
{
    background: #E2E8F0;
}

QPushButton[active="true"]
{
    background: #2563EB;
    color: #FFFFFF;
    border: none;
}

QPushButton[variant="success"]
{
    background: #16A34A;
    color: #FFFFFF;
    border: none;
}

QPushButton[variant="danger"]
{
    background: #DC2626;
    color: #FFFFFF;
    border: none;
}

QPushButton[variant="warning"]
{
    background: #F59E0B;
    color: #111827;
    border: none;
}

QPushButton:disabled
{
    background: #F1F5F9;
    color: #64748B;
    border: 1px solid #E2E8F0;
}

QLineEdit
{
    border: 1px solid #CBD5E1;
    border-radius: 6px;
    padding: 5px 7px;
    background: #FFFFFF;
}

QComboBox,
QDoubleSpinBox
{
    border: 1px solid #CBD5E1;
    border-radius: 6px;
    padding: 5px 8px;
    background: #FFFFFF;
    min-height: 24px;
}

QComboBox:disabled,
QDoubleSpinBox:disabled
{
    background: #F1F5F9;
    color: #94A3B8;
}

QTextEdit
{
    border: 1px solid #CBD5E1;
    border-radius: 8px;
    background: #0F172A;
    color: #E2E8F0;
    font-family: Consolas, monospace;
    font-size: 9pt;
}

QProgressBar
{
    border: none;
    border-radius: 6px;
    background: #E2E8F0;
    min-height: 10px;
    text-align: center;
}

QProgressBar::chunk
{
    border-radius: 6px;
    background: #14B8A6;
}

QStatusBar
{
    background: #F8FAFC;
    color: #475569;
}
"""
